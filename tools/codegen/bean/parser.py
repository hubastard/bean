"""Parse Apple SDK headers via clang AST JSON."""

from __future__ import annotations

import copy
import json
import re
import subprocess
from dataclasses import dataclass, field
from typing import Any, Optional

from .typemap import CType, qual_type_to_c, sanitize_qual_type


@dataclass
class ParsedMethod:
    class_name: str
    selector: str
    is_class_method: bool
    return_type: CType
    params: list[tuple[str, CType]]
    category: Optional[str] = None
    availability: Optional[str] = None


@dataclass
class ParsedProperty:
    class_name: str
    name: str
    type: CType
    readonly: bool
    category: Optional[str] = None


@dataclass
class ParsedEnumConstant:
    name: str
    value: Optional[str] = None


@dataclass
class ParsedStructField:
    name: str
    type_spelling: str


@dataclass
class ParsedStruct:
    name: str
    fields: list[ParsedStructField]


@dataclass
class ParsedEnum:
    name: str
    constants: list[ParsedEnumConstant]
    underlying: str = "NSInteger"


@dataclass
class ParseResult:
    methods: list[ParsedMethod] = field(default_factory=list)
    properties: list[ParsedProperty] = field(default_factory=list)
    enums: list[ParsedEnum] = field(default_factory=list)
    structs: list[ParsedStruct] = field(default_factory=list)
    errors: list[str] = field(default_factory=list)


_PARSE_CACHE: dict[tuple[str, str], ParseResult] = {}


def _run_clang_ast(sdk_path: str, header_path: str, _framework: str) -> dict[str, Any]:
    args = [
        "xcrun",
        "clang",
        "-x",
        "objective-c",
        "-isysroot",
        sdk_path,
        "-iframework",
        f"{sdk_path}/System/Library/Frameworks",
        "-F",
        f"{sdk_path}/System/Library/Frameworks",
        "-fblocks",
        "-fapinotes-modules",
        "-Wno-everything",
        "-Xclang",
        "-ast-dump=json",
        "-fsyntax-only",
        header_path,
    ]
    proc = subprocess.run(args, capture_output=True, text=True)
    if proc.returncode not in (0, 74):
        raise RuntimeError(f"clang failed ({proc.returncode}): {proc.stderr[:500]}")
    try:
        return json.loads(proc.stdout)
    except json.JSONDecodeError as exc:
        raise RuntimeError(f"Invalid AST JSON: {exc}\n{proc.stderr[:500]}") from exc


def _parse_category_name(node: dict[str, Any]) -> Optional[str]:
    # ObjCCategoryDecl may expose category name via mangled children; fallback None.
    return node.get("name") if node.get("kind") == "ObjCCategoryDecl" else None


def _extract_params(method_node: dict[str, Any]) -> list[tuple[str, CType]]:
    params: list[tuple[str, CType]] = []
    for child in method_node.get("inner", []):
        if child.get("kind") != "ParmVarDecl":
            continue
        name = child.get("name") or "arg"
        qual = child.get("type", {}).get("desugaredQualType") or child.get("type", {}).get("qualType", "id")
        params.append((name, qual_type_to_c(qual)))
    return params


def _format_field_decl(field_type: str, field_name: str) -> str:
    if field_type.endswith("]") and "[" in field_type:
        base, bracket = field_type.rsplit("[", 1)
        return f"{base.strip()} {field_name}[{bracket}"
    return f"{field_type} {field_name}"


def _record_fields(record_node: dict[str, Any]) -> list[ParsedStructField]:
    fields: list[ParsedStructField] = []
    for child in record_node.get("inner", []):
        if child.get("kind") != "FieldDecl":
            continue
        name = child.get("name")
        if not name or name.startswith("__"):
            continue
        qual = child.get("type", {}).get("desugaredQualType") or child.get("type", {}).get("qualType", "void")
        fields.append(ParsedStructField(name=name, type_spelling=sanitize_qual_type(qual)))
    return fields


def _struct_is_emittable(struct: ParsedStruct, used_types: set[str]) -> bool:
    if struct.name not in used_types:
        return False
    if struct.name in {
        "NSRange",
        "NSPoint",
        "NSSize",
        "NSRect",
        "NSEdgeInsets",
        "NSDirectionalEdgeInsets",
        "CGRect",
        "CGPoint",
        "CGSize",
        "_NSRect",
        "_NSPoint",
        "_NSSize",
        "CLLocationCoordinate2D",
        "CAFrameRateRange",
    }:
        return False
    for field in struct.fields:
        text = field.type_spelling
        if "(*" in text or "(*" in field.name:
            return False
        if text.startswith("struct (unnamed"):
            return False
    return True


def _index_nodes(node: dict[str, Any], index: dict[str, dict[str, Any]]) -> None:
    node_id = node.get("id")
    if node_id:
        index[node_id] = node
    for child in node.get("inner", []):
        _index_nodes(child, index)


def _record_fields_from_index(record_id: str, index: dict[str, dict[str, Any]]) -> list[ParsedStructField]:
    record_node = index.get(record_id)
    if not record_node:
        return []
    return _record_fields(record_node)


def _extract_struct_from_typedef(node: dict[str, Any], index: dict[str, dict[str, Any]]) -> ParsedStruct | None:
    name = node.get("name")
    if not name or name.startswith("_"):
        return None
    for child in node.get("inner", []):
        record_id = None
        if child.get("kind") == "RecordType":
            decl = child.get("decl") or {}
            record_id = decl.get("id")
        elif child.get("kind") == "ElaboratedType":
            owned = child.get("ownedTagDecl") or {}
            record_id = owned.get("id")
        if record_id:
            fields = _record_fields_from_index(record_id, index)
            if fields:
                return ParsedStruct(name=name, fields=fields)
    return None


def _enum_underlying(enum_node: dict[str, Any]) -> str:
    for child in enum_node.get("inner", []):
        if child.get("kind") == "EnumDecl":
            continue
        qual = child.get("type", {}).get("desugaredQualType") or child.get("type", {}).get("qualType")
        if qual:
            mapped = qual_type_to_c(qual)
            return mapped.spelling
    return "NSInteger"


def _find_constant_value(node: dict[str, Any]) -> Optional[str]:
    if node.get("kind") == "ConstantExpr" and "value" in node:
        return str(node["value"])
    for child in node.get("inner", []):
        found = _find_constant_value(child)
        if found is not None:
            return found
    return None


def _extract_enum_constants(enum_node: dict[str, Any]) -> list[ParsedEnumConstant]:
    constants: list[ParsedEnumConstant] = []
    for child in enum_node.get("inner", []):
        if child.get("kind") != "EnumConstantDecl":
            continue
        name = child.get("name")
        if not name:
            continue
        value = _find_constant_value(child)
        constants.append(ParsedEnumConstant(name=name, value=value))
    return constants


def _walk(
    node: dict[str, Any],
    class_name: Optional[str],
    category: Optional[str],
    result: ParseResult,
    index: dict[str, dict[str, Any]],
) -> None:
    kind = node.get("kind")
    if kind == "TypedefDecl" and node.get("name"):
        parsed_struct = _extract_struct_from_typedef(node, index)
        if parsed_struct:
            result.structs.append(parsed_struct)
    elif kind == "EnumDecl" and node.get("name"):
        constants = _extract_enum_constants(node)
        if constants:
            result.enums.append(
                ParsedEnum(
                    name=node["name"],
                    constants=constants,
                    underlying=_enum_underlying(node),
                )
            )
    elif kind == "TypedefDecl" and node.get("name"):
        inner = node.get("inner") or []
        if inner and inner[0].get("kind") == "EnumType":
            # typedef enum Foo Foo; — constants appear on a separate EnumDecl, skip duplicate shell.
            pass
    if kind == "ObjCInterfaceDecl":
        class_name = node.get("name")
        category = None
    elif kind == "ObjCCategoryDecl":
        category = node.get("name")
        interface = node.get("interface") or {}
        if interface.get("name"):
            class_name = interface["name"]
    elif kind == "ObjCMethodDecl" and class_name:
        selector = node.get("name", "")
        if not selector:
            return
        return_qual = node.get("returnType", {}).get("desugaredQualType") or node.get("returnType", {}).get("qualType", "void")
        method = ParsedMethod(
            class_name=class_name,
            selector=selector,
            is_class_method=not node.get("instance", True),
            return_type=qual_type_to_c(return_qual),
            params=_extract_params(node),
            category=category,
        )
        result.methods.append(method)
    elif kind == "ObjCPropertyDecl" and class_name:
        prop_type = node.get("type", {}).get("desugaredQualType") or node.get("type", {}).get("qualType", "id")
        result.properties.append(
            ParsedProperty(
                class_name=class_name,
                name=node.get("name", ""),
                type=qual_type_to_c(prop_type),
                readonly=bool(node.get("readonly")),
                category=category,
            )
        )

    for child in node.get("inner", []):
        _walk(child, class_name, category, result, index)


def clear_parse_cache() -> None:
    _PARSE_CACHE.clear()


def _parse_header_uncached(sdk_path: str, umbrella_relative: str) -> ParseResult:
    # umbrella_relative like "Foundation/Foundation.h" -> Framework.framework/Headers/Foundation.h
    framework, header_name = umbrella_relative.split("/", 1)
    header_path = f"{sdk_path}/System/Library/Frameworks/{framework}.framework/Headers/{header_name}"
    ast = _run_clang_ast(sdk_path, header_path, framework)
    index: dict[str, dict[str, Any]] = {}
    _index_nodes(ast, index)
    result = ParseResult()
    _walk(ast, None, None, result, index)
    return result


def parse_header(sdk_path: str, umbrella_relative: str, *, copy_result: bool = False) -> ParseResult:
    key = (sdk_path, umbrella_relative)
    if key not in _PARSE_CACHE:
        _PARSE_CACHE[key] = _parse_header_uncached(sdk_path, umbrella_relative)
    result = _PARSE_CACHE[key]
    return copy.deepcopy(result) if copy_result else result


def dedupe_structs(structs: list[ParsedStruct]) -> list[ParsedStruct]:
    by_name: dict[str, ParsedStruct] = {}
    for struct in structs:
        if struct.name not in by_name or len(struct.fields) > len(by_name[struct.name].fields):
            by_name[struct.name] = struct
    return list(by_name.values())


def dedupe_enums(enums: list[ParsedEnum]) -> list[ParsedEnum]:
    by_name: dict[str, ParsedEnum] = {}
    for enum in enums:
        existing = by_name.get(enum.name)
        if not existing:
            by_name[enum.name] = enum
            continue
        known = {c.name for c in existing.constants}
        for constant in enum.constants:
            if constant.name not in known:
                existing.constants.append(constant)
                known.add(constant.name)
    return list(by_name.values())


def dedupe_methods(methods: list[ParsedMethod]) -> list[ParsedMethod]:
    seen: set[tuple[str, str, bool]] = set()
    out: list[ParsedMethod] = []
    for method in methods:
        key = (method.class_name, method.selector, method.is_class_method)
        if key in seen:
            continue
        seen.add(key)
        out.append(method)
    return out


def dedupe_properties(properties: list[ParsedProperty]) -> list[ParsedProperty]:
    seen: set[tuple[str, str]] = set()
    out: list[ParsedProperty] = []
    for prop in properties:
        key = (prop.class_name, prop.name)
        if key in seen:
            continue
        seen.add(key)
        out.append(prop)
    return out
