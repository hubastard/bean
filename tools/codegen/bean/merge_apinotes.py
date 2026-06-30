"""Apply apinotes annotations to parsed AST results."""

from __future__ import annotations

from dataclasses import dataclass

from .apinotes import APINotesIndex
from .parser import ParsedMethod, ParsedProperty, ParseResult
from .typemap import CType, qual_type_to_c


@dataclass
class ApinotesSkip:
    class_name: str
    selector: str
    kind: str  # "method" | "property"


def _apply_method_note(method: ParsedMethod, note) -> ParsedMethod:
    if note.result_type:
        method.return_type = qual_type_to_c(note.result_type)
    if note.param_types:
        updated: list[tuple[str, CType]] = []
        for index, (name, ctype) in enumerate(method.params):
            override = note.param_types.get(index)
            if override:
                updated.append((name, qual_type_to_c(override)))
            else:
                updated.append((name, ctype))
        method.params = updated
    return method


def apply_apinotes(
    parse_result: ParseResult,
    apinotes: APINotesIndex | None,
) -> tuple[ParseResult, list[ApinotesSkip]]:
    skipped: list[ApinotesSkip] = []
    if apinotes is None:
        return parse_result, skipped

    filtered_methods: list[ParsedMethod] = []
    for method in parse_result.methods:
        if apinotes.is_nonswift_method(method.class_name, method.selector, method.is_class_method):
            skipped.append(ApinotesSkip(method.class_name, method.selector, "method"))
            continue
        note = apinotes.method_note(method.class_name, method.selector, method.is_class_method)
        if note:
            method = _apply_method_note(method, note)
        filtered_methods.append(method)
    parse_result.methods = filtered_methods

    filtered_properties: list[ParsedProperty] = []
    for prop in parse_result.properties:
        if apinotes.is_nonswift_property(prop.class_name, prop.name, False) or apinotes.is_nonswift_property(
            prop.class_name, prop.name, True
        ):
            skipped.append(ApinotesSkip(prop.class_name, prop.name, "property"))
            continue
        note = apinotes.property_note(prop.class_name, prop.name, False) or apinotes.property_note(
            prop.class_name, prop.name, True
        )
        if note and note.type_override:
            prop.type = qual_type_to_c(note.type_override)
        filtered_properties.append(prop)
    parse_result.properties = filtered_properties

    parse_result.enums = [
        enum
        for enum in parse_result.enums
        if enum.name not in apinotes.nonswift_typedefs
    ]
    for enum in parse_result.enums:
        enum.constants = [
            constant
            for constant in enum.constants
            if constant.name not in apinotes.nonswift_enumerators
        ]
    parse_result.enums = [enum for enum in parse_result.enums if enum.constants]

    return parse_result, skipped


def enrich_docs_from_apinotes(docs, apinotes: APINotesIndex | None) -> None:
    """Fill missing docs with apinotes AvailabilityMsg one-liners."""
    if apinotes is None:
        return
    for key, note in apinotes.methods.items():
        if not note.availability_msg or key in docs.methods:
            continue
        docs.methods[key] = note.availability_msg
    for key, note in apinotes.properties.items():
        class_name, name, _is_class = key
        prop_key = (class_name, name)
        if not note.availability_msg or prop_key in docs.properties:
            continue
        docs.properties[prop_key] = note.availability_msg
