#!/usr/bin/env python3
"""Generate bean framework headers from Apple SDK AST."""

from __future__ import annotations

import argparse
import subprocess
import sys
from pathlib import Path

from bean.apinotes import framework_name_from_umbrella, load_apinotes
from bean.config import FRAMEWORK_BY_NAME, GENERATION_ORDER
from bean.docs import load_framework_docs
from bean.emit_c import UnsupportedEntry, emit_framework_header, emit_unsupported_manifest
from bean.merge_apinotes import apply_apinotes, enrich_docs_from_apinotes
from bean.parser import clear_parse_cache, dedupe_enums, dedupe_structs, parse_header

_SKIP_NAMES_CACHE: dict[tuple[str, str], tuple[set[str], set[str]]] = {}


def clear_generation_caches() -> None:
    clear_parse_cache()
    _SKIP_NAMES_CACHE.clear()


def default_sdk_path() -> str:
    return subprocess.check_output(["xcrun", "--show-sdk-path"], text=True).strip()


def repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def dependency_skip_names(name: str, sdk_path: str) -> tuple[set[str], set[str]]:
    key = (sdk_path, name)
    cached = _SKIP_NAMES_CACHE.get(key)
    if cached is not None:
        return cached

    spec = FRAMEWORK_BY_NAME[name]
    skip_types: set[str] = set()
    skip_functions: set[str] = set()
    for dep in spec.depends_on:
        dep_spec = FRAMEWORK_BY_NAME[dep]
        dep_parsed = parse_header(sdk_path, dep_spec.umbrella)
        skip_types |= {e.name for e in dedupe_enums(dep_parsed.enums)}
        skip_types |= {s.name for s in dedupe_structs(dep_parsed.structs)}
        from bean.emit_c import c_function_name
        from bean.parser import dedupe_methods, dedupe_properties

        for method in dedupe_methods(dep_parsed.methods):
            skip_functions.add(c_function_name(method.class_name, method.selector, method.is_class_method))
        for prop in dedupe_properties(dep_parsed.properties):
            skip_functions.add(c_function_name(prop.class_name, prop.name, False))
            if not prop.readonly:
                skip_functions.add(c_function_name(prop.class_name, f"set{prop.name[0].upper()}{prop.name[1:] if len(prop.name) > 1 else ''}:", False))
        dep_types, dep_funcs = dependency_skip_names(dep, sdk_path)
        skip_types |= dep_types
        skip_functions |= dep_funcs
    result = (skip_types, skip_functions)
    _SKIP_NAMES_CACHE[key] = result
    return result


def generate_framework(
    name: str,
    sdk_path: str,
    out_dir: Path,
    use_apinotes: bool = True,
    with_docs: bool = False,
) -> list:
    spec = FRAMEWORK_BY_NAME[name]
    print(f"Generating {spec.name}.h from {spec.umbrella}...")
    parsed = parse_header(sdk_path, spec.umbrella, copy_result=True)
    unsupported_from_notes: list[UnsupportedEntry] = []
    framework = framework_name_from_umbrella(spec.umbrella)
    notes = load_apinotes(sdk_path, framework) if use_apinotes else None
    if use_apinotes and notes is not None:
        parsed, skipped = apply_apinotes(parsed, notes)
        for entry in skipped:
            unsupported_from_notes.append(
                UnsupportedEntry(
                    class_name=entry.class_name,
                    selector=entry.selector,
                    reason="apinotes nonswift",
                    framework=spec.name,
                )
            )
        if skipped:
            print(f"  apinotes filtered {len(skipped)} declarations")
    docs = None
    if with_docs:
        docs = load_framework_docs(sdk_path, framework)
        if use_apinotes and notes is not None:
            enrich_docs_from_apinotes(docs, notes)
        print(
            f"  docs: {len(docs.classes)} classes, {len(docs.methods)} methods, "
            f"{len(docs.properties)} properties"
        )
    skip_types, skip_functions = dependency_skip_names(name, sdk_path)
    header, unsupported = emit_framework_header(
        spec, parsed, skip_types, skip_functions, docs=docs, with_docs=with_docs
    )
    out_path = out_dir / f"{spec.name}.h"
    out_path.write_text(header, encoding="utf-8")
    print(f"  wrote {out_path} ({len(parsed.methods)} methods, {len(parsed.properties)} properties)")
    return unsupported_from_notes + unsupported


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Generate bean Cocoa C headers")
    parser.add_argument("--sdk", default=default_sdk_path(), help="macOS SDK path")
    parser.add_argument(
        "--framework",
        action="append",
        dest="frameworks",
        help="Framework to generate (repeatable). Default: all configured frameworks.",
    )
    parser.add_argument(
        "--output",
        type=Path,
        default=repo_root() / "include" / "bean",
        help="Output directory for generated headers",
    )
    parser.add_argument(
        "--no-apinotes",
        action="store_true",
        help="Disable .apinotes filtering and type overrides.",
    )
    parser.add_argument(
        "--with-docs",
        action="store_true",
        help="Embed SDK header comments and @see links above generated wrappers.",
    )
    args = parser.parse_args(argv)

    out_dir: Path = args.output
    out_dir.mkdir(parents=True, exist_ok=True)

    selected = args.frameworks or list(GENERATION_ORDER)
    clear_generation_caches()
    unsupported_all = []
    for name in selected:
        if name not in FRAMEWORK_BY_NAME:
            print(f"Unknown framework: {name}", file=sys.stderr)
            return 1
        unsupported_all.extend(
            generate_framework(
                name,
                args.sdk,
                out_dir,
                use_apinotes=not args.no_apinotes,
                with_docs=args.with_docs,
            )
        )

    if not args.frameworks:
        manifest = emit_unsupported_manifest(unsupported_all)
        manifest_path = out_dir / "unsupported.h"
        manifest_path.write_text(manifest, encoding="utf-8")
        print(f"Wrote unsupported manifest: {manifest_path} ({len(unsupported_all)} entries)")

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
