"""Extract documentation comments from Apple SDK framework headers."""

from __future__ import annotations

import re
from dataclasses import dataclass, field
from pathlib import Path


INTERFACE_RE = re.compile(r"@interface\s+(\w+)")
END_RE = re.compile(r"@end\b")
PROPERTY_RE = re.compile(r"@property\b.*\b(\w+)\s*;\s*$")
METHOD_RE = re.compile(r"[-+]\s*\([^)]+\)\s*(\w+(?::[^;{]*)?)\s*[;{]")
BLOCK_COMMENT_START_RE = re.compile(r"/\*")
BLOCK_COMMENT_END_RE = re.compile(r"\*/")


@dataclass
class DocsIndex:
    classes: dict[str, str] = field(default_factory=dict)
    methods: dict[tuple[str, str, bool], str] = field(default_factory=dict)
    properties: dict[tuple[str, str], str] = field(default_factory=dict)

    def method_doc(self, class_name: str, selector: str, is_class_method: bool) -> str | None:
        return self.methods.get((class_name, selector, is_class_method))

    def property_doc(self, class_name: str, name: str) -> str | None:
        return self.properties.get((class_name, name))

    def class_doc(self, class_name: str) -> str | None:
        return self.classes.get(class_name)


def framework_headers_dir(sdk_path: str, framework: str) -> Path | None:
    root = Path(sdk_path) / "System/Library/Frameworks" / f"{framework}.framework"
    if not root.exists():
        return None
    for candidate in root.glob("**/Headers"):
        if candidate.is_dir():
            return candidate
    return None


def _normalize_comment(lines: list[str], max_len: int = 1200) -> str:
    cleaned: list[str] = []
    for line in lines:
        text = line.strip()
        if text.startswith("///"):
            text = text[3:].strip()
        if text.startswith("/*"):
            text = text[2:].strip()
        if text.endswith("*/"):
            text = text[:-2].strip()
        text = text.replace("\\c ", "").replace("\\c", "")
        text = re.sub(r"`([^`]+)`", r"\1", text)
        text = text.replace("*/", "* /")
        if text:
            cleaned.append(text)
    body = " ".join(cleaned)
    body = re.sub(r"\s+", " ", body).strip()
    if len(body) > max_len:
        body = body[: max_len - 3].rstrip() + "..."
    return body


def _method_selector(raw: str) -> str:
    selector = raw.strip()
    if not selector.endswith(":") and ":" in selector:
        selector = selector.rstrip(":") + ":"
    return selector


def _parse_header_text(text: str, index: DocsIndex) -> None:
    current_class: str | None = None
    pending_lines: list[str] = []
    in_block = False
    block_lines: list[str] = []

    def pending_comment() -> str | None:
        if not pending_lines:
            return None
        return _normalize_comment(pending_lines)

    def clear_pending() -> None:
        pending_lines.clear()

    def consume_comment_for(key_maker):
        comment = pending_comment()
        clear_pending()
        if comment and current_class:
            key_maker(comment)

    for raw_line in text.splitlines():
        line = raw_line.rstrip("\n")
        stripped = line.strip()

        if in_block:
            block_lines.append(line)
            if BLOCK_COMMENT_END_RE.search(line):
                in_block = False
                pending_lines = block_lines[:]
                block_lines = []
            continue

        if stripped.startswith("///"):
            pending_lines.append(stripped)
            continue

        if "/*" in stripped:
            if stripped.startswith("/*") and "*/" in stripped:
                pending_lines = [stripped]
            elif stripped.startswith("/*"):
                in_block = True
                block_lines = [line]
            continue

        if not stripped:
            continue

        interface = INTERFACE_RE.search(stripped)
        if interface:
            consume_comment_for(lambda c: index.classes.setdefault(interface.group(1), c))
            current_class = interface.group(1)
            continue

        if END_RE.search(stripped):
            current_class = None
            clear_pending()
            continue

        if current_class is None:
            clear_pending()
            continue

        prop = PROPERTY_RE.search(stripped)
        if prop:
            prop_name = prop.group(1)

            def store_prop(comment: str, name: str = prop_name) -> None:
                index.properties[(current_class, name)] = comment

            consume_comment_for(store_prop)
            continue

        method = METHOD_RE.search(stripped)
        if method:
            selector = _method_selector(method.group(1))
            is_class = stripped.lstrip().startswith("+")

            def store_method(comment: str, sel: str = selector, cls_method: bool = is_class) -> None:
                index.methods[(current_class, sel, cls_method)] = comment

            consume_comment_for(store_method)
            continue

        clear_pending()


def load_framework_docs(sdk_path: str, framework: str) -> DocsIndex:
    index = DocsIndex()
    headers_dir = framework_headers_dir(sdk_path, framework)
    if headers_dir is None:
        return index
    for header_path in sorted(headers_dir.glob("*.h")):
        try:
            text = header_path.read_text(encoding="utf-8", errors="replace")
        except OSError:
            continue
        _parse_header_text(text, index)
    return index


def objc_method_see(class_name: str, selector: str, is_class_method: bool) -> str:
    prefix = "+" if is_class_method else "-"
    return f"{prefix}[{class_name} {selector.rstrip(':')}{':' if selector.endswith(':') or ':' in selector else ''}]"


def objc_property_see(class_name: str, name: str) -> str:
    return f"-[{class_name} {name}]"


def lookup_method_doc(
    docs: DocsIndex | None,
    class_name: str,
    selector: str,
    is_class_method: bool,
) -> str | None:
    if docs is None:
        return None
    doc = docs.method_doc(class_name, selector, is_class_method)
    if doc:
        return doc
    if ":" not in selector:
        return docs.property_doc(class_name, selector)
    return None


def lookup_property_doc(docs: DocsIndex | None, class_name: str, name: str) -> str | None:
    if docs is None:
        return None
    doc = docs.property_doc(class_name, name)
    if doc:
        return doc
    return docs.method_doc(class_name, name, False)


def format_doc_comment(body: str | None, see: str | None = None) -> str:
    lines = ["/**"]
    if body:
        words = body.split()
        line = " * "
        for word in words:
            candidate = line + word + " "
            if len(candidate) > 96:
                lines.append(line.rstrip())
                line = f" * {word} "
            else:
                line = candidate
        if line.strip() != "*":
            lines.append(line.rstrip())
    if see:
        lines.append(f" * @see {see}")
    lines.append(" */")
    return "\n".join(lines)
