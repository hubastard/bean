"""Load Apple SDK .apinotes YAML and build lookup tables for bean-gen."""

from __future__ import annotations

from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Optional

try:
    import yaml
except ImportError:  # pragma: no cover
    yaml = None  # type: ignore[assignment]


@dataclass
class MethodNote:
    availability: Optional[str] = None
    availability_msg: Optional[str] = None
    result_type: Optional[str] = None
    param_types: dict[int, str] = field(default_factory=dict)


@dataclass
class PropertyNote:
    availability: Optional[str] = None
    availability_msg: Optional[str] = None
    type_override: Optional[str] = None
    is_class_property: bool = False


@dataclass
class APINotesIndex:
    """Merged apinotes for one framework module."""

    methods: dict[tuple[str, str, bool], MethodNote] = field(default_factory=dict)
    properties: dict[tuple[str, str, bool], PropertyNote] = field(default_factory=dict)
    nonswift_globals: set[str] = field(default_factory=set)
    nonswift_typedefs: set[str] = field(default_factory=set)
    nonswift_enumerators: set[str] = field(default_factory=set)

    def method_note(self, class_name: str, selector: str, is_class_method: bool) -> Optional[MethodNote]:
        return self.methods.get((class_name, selector, is_class_method))

    def property_note(self, class_name: str, name: str, is_class_property: bool) -> Optional[PropertyNote]:
        return self.properties.get((class_name, name, is_class_property))

    def is_nonswift_method(self, class_name: str, selector: str, is_class_method: bool) -> bool:
        note = self.method_note(class_name, selector, is_class_method)
        return note is not None and note.availability == "nonswift"

    def is_nonswift_property(self, class_name: str, name: str, is_class_property: bool) -> bool:
        note = self.property_note(class_name, name, is_class_property)
        return note is not None and note.availability == "nonswift"


def _is_class_kind(kind: str | None) -> bool:
    return (kind or "").lower() == "class"


def _merge_availability(current: Optional[str], incoming: Optional[str]) -> Optional[str]:
    if incoming is None:
        return current
    if incoming == "available":
        return None
    return incoming


def _ingest_method(
    class_name: str,
    entry: dict[str, Any],
    index: APINotesIndex,
) -> None:
    selector = entry.get("Selector")
    if not selector:
        return
    is_class = _is_class_kind(entry.get("MethodKind"))
    key = (class_name, selector, is_class)
    note = index.methods.setdefault(key, MethodNote())
    note.availability = _merge_availability(note.availability, entry.get("Availability"))
    if entry.get("AvailabilityMsg"):
        note.availability_msg = entry["AvailabilityMsg"]
    if entry.get("ResultType"):
        note.result_type = entry["ResultType"]
    for param in entry.get("Parameters") or []:
        if not isinstance(param, dict):
            continue
        position = param.get("Position")
        param_type = param.get("Type")
        if position is None or param_type is None:
            continue
        note.param_types[int(position)] = param_type


def _ingest_property(
    class_name: str,
    entry: dict[str, Any],
    index: APINotesIndex,
) -> None:
    name = entry.get("Name")
    if not name:
        return
    is_class = _is_class_kind(entry.get("PropertyKind"))
    key = (class_name, name, is_class)
    note = index.properties.setdefault(key, PropertyNote(is_class_property=is_class))
    note.availability = _merge_availability(note.availability, entry.get("Availability"))
    if entry.get("AvailabilityMsg"):
        note.availability_msg = entry["AvailabilityMsg"]
    if entry.get("Type"):
        note.type_override = entry["Type"]


def _ingest_named_decl(section: str, entry: dict[str, Any], index: APINotesIndex) -> None:
    name = entry.get("Name")
    if not name:
        return
    availability = entry.get("Availability")
    if availability != "nonswift":
        return
    if section in {"Globals", "Typedefs", "Enumerators", "Tags"}:
        if section == "Globals":
            index.nonswift_globals.add(name)
        elif section == "Enumerators":
            index.nonswift_enumerators.add(name)
        else:
            index.nonswift_typedefs.add(name)


def _ingest_classes(classes: list[Any] | None, index: APINotesIndex) -> None:
    if not classes:
        return
    for cls in classes:
        if not isinstance(cls, dict):
            continue
        class_name = cls.get("Name")
        if not class_name:
            continue
        if cls.get("Availability") == "nonswift":
            # Class-level nonswift is rare; skip all methods via a sentinel if needed later.
            pass
        for method in cls.get("Methods") or []:
            if isinstance(method, dict):
                _ingest_method(class_name, method, index)
        for prop in cls.get("Properties") or []:
            if isinstance(prop, dict):
                _ingest_property(class_name, prop, index)


def _ingest_section(section: str, entries: list[Any] | None, index: APINotesIndex) -> None:
    if not entries:
        return
    if section in {"Classes", "Protocols"}:
        _ingest_classes(entries, index)
        return
    if section == "Tags":
        for entry in entries:
            if isinstance(entry, dict):
                _ingest_named_decl("Tags", entry, index)
                for nested in entry.get("Tags") or []:
                    if isinstance(nested, dict):
                        _ingest_named_decl("Tags", nested, index)
        return
    for entry in entries:
        if isinstance(entry, dict):
            _ingest_named_decl(section, entry, index)


def _ingest_document(doc: dict[str, Any], index: APINotesIndex) -> None:
    for section in ("Classes", "Protocols", "Globals", "Typedefs", "Enumerators", "Tags"):
        _ingest_section(section, doc.get(section), index)
    for version_block in doc.get("SwiftVersions") or []:
        if isinstance(version_block, dict):
            _ingest_document(version_block, index)


def find_apinotes_path(sdk_path: str, framework: str) -> Optional[Path]:
    framework_dir = Path(sdk_path) / "System/Library/Frameworks" / f"{framework}.framework"
    if not framework_dir.exists():
        return None
    matches = sorted(framework_dir.glob(f"**/Headers/{framework}.apinotes"))
    for path in matches:
        if "_private" in path.name:
            continue
        return path
    return None


def load_apinotes(sdk_path: str, framework: str) -> APINotesIndex:
    """Load and merge apinotes for a framework module."""
    index = APINotesIndex()
    path = find_apinotes_path(sdk_path, framework)
    if path is None:
        return index
    if yaml is None:
        raise RuntimeError("PyYAML is required for apinotes support: pip install pyyaml")
    with path.open(encoding="utf-8") as handle:
        docs = list(yaml.safe_load_all(handle))
    for doc in docs:
        if isinstance(doc, dict):
            _ingest_document(doc, index)
    return index


def framework_name_from_umbrella(umbrella: str) -> str:
    return umbrella.split("/", 1)[0]
