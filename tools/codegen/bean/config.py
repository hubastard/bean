"""Framework generation configuration."""

from __future__ import annotations

from dataclasses import dataclass
from typing import Optional


@dataclass(frozen=True)
class FrameworkSpec:
    name: str
    umbrella: str
    guard: str
    depends_on: tuple[str, ...] = ()
    link_symbol: Optional[str] = None
    emit_parsed_types: bool = True


FRAMEWORKS: tuple[FrameworkSpec, ...] = (
    FrameworkSpec(
        name="foundation",
        umbrella="Foundation/Foundation.h",
        guard="BEAN_FOUNDATION_H",
        depends_on=(),
    ),
    FrameworkSpec(
        name="appkit",
        umbrella="AppKit/AppKit.h",
        guard="BEAN_APPKIT_H",
        depends_on=("foundation", "quartzcore"),
        link_symbol="NSApp",
    ),
    FrameworkSpec(
        name="coregraphics",
        umbrella="CoreGraphics/CoreGraphics.h",
        guard="BEAN_COREGRAPHICS_H",
        depends_on=(),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="corefoundation",
        umbrella="CoreFoundation/CoreFoundation.h",
        guard="BEAN_COREFOUNDATION_H",
        depends_on=(),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="quartzcore",
        umbrella="QuartzCore/QuartzCore.h",
        guard="BEAN_QUARTZCORE_H",
        depends_on=("coregraphics", "foundation", "metal"),
    ),
    FrameworkSpec(
        name="webkit",
        umbrella="WebKit/WebKit.h",
        guard="BEAN_WEBKIT_H",
        depends_on=("appkit", "foundation"),
    ),
    FrameworkSpec(
        name="avfoundation",
        umbrella="AVFoundation/AVFoundation.h",
        guard="BEAN_AVFOUNDATION_H",
        depends_on=("foundation", "coremedia", "corevideo", "metal"),
    ),
    FrameworkSpec(
        name="coremedia",
        umbrella="CoreMedia/CoreMedia.h",
        guard="BEAN_COREMEDIA_H",
        depends_on=("corefoundation", "foundation"),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="corevideo",
        umbrella="CoreVideo/CoreVideo.h",
        guard="BEAN_COREVIDEO_H",
        depends_on=("corefoundation", "coregraphics", "foundation"),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="coredata",
        umbrella="CoreData/CoreData.h",
        guard="BEAN_COREDATA_H",
        depends_on=("foundation", "cloudkit"),
    ),
    FrameworkSpec(
        name="coretext",
        umbrella="CoreText/CoreText.h",
        guard="BEAN_CORETEXT_H",
        depends_on=("corefoundation", "coregraphics"),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="imageio",
        umbrella="ImageIO/ImageIO.h",
        guard="BEAN_IMAGEIO_H",
        depends_on=("corefoundation", "coregraphics"),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="iokit",
        umbrella="IOKit/IOKitLib.h",
        guard="BEAN_IOKIT_H",
        depends_on=("corefoundation",),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="security",
        umbrella="Security/Security.h",
        guard="BEAN_SECURITY_H",
        depends_on=("corefoundation",),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="carbon",
        umbrella="Carbon/Carbon.h",
        guard="BEAN_CARBON_H",
        depends_on=("corefoundation",),
        emit_parsed_types=False,
    ),
    FrameworkSpec(
        name="scenekit",
        umbrella="SceneKit/SceneKit.h",
        guard="BEAN_SCENEKIT_H",
        depends_on=("appkit", "quartzcore", "foundation"),
    ),
    FrameworkSpec(
        name="metal",
        umbrella="Metal/Metal.h",
        guard="BEAN_METAL_H",
        depends_on=("foundation",),
    ),
    FrameworkSpec(
        name="metalkit",
        umbrella="MetalKit/MetalKit.h",
        guard="BEAN_METALKIT_H",
        depends_on=("metal", "appkit"),
    ),
    FrameworkSpec(
        name="gameplaykit",
        umbrella="GameplayKit/GameplayKit.h",
        guard="BEAN_GAMEPLAYKIT_H",
        depends_on=("foundation", "scenekit"),
    ),
    FrameworkSpec(
        name="mapkit",
        umbrella="MapKit/MapKit.h",
        guard="BEAN_MAPKIT_H",
        depends_on=("appkit", "foundation"),
    ),
    FrameworkSpec(
        name="pdfkit",
        umbrella="PDFKit/PDFKit.h",
        guard="BEAN_PDFKIT_H",
        depends_on=("appkit", "foundation", "cloudkit"),
    ),
    FrameworkSpec(
        name="storekit",
        umbrella="StoreKit/StoreKit.h",
        guard="BEAN_STOREKIT_H",
        depends_on=("appkit", "foundation"),
    ),
    FrameworkSpec(
        name="spritekit",
        umbrella="SpriteKit/SpriteKit.h",
        guard="BEAN_SPRITEKIT_H",
        depends_on=("appkit", "quartzcore", "foundation", "cloudkit"),
    ),
    FrameworkSpec(
        name="cloudkit",
        umbrella="CloudKit/CloudKit.h",
        guard="BEAN_CLOUDKIT_H",
        depends_on=("foundation",),
    ),
    FrameworkSpec(
        name="contacts",
        umbrella="Contacts/Contacts.h",
        guard="BEAN_CONTACTS_H",
        depends_on=("foundation",),
    ),
    FrameworkSpec(
        name="eventkit",
        umbrella="EventKit/EventKit.h",
        guard="BEAN_EVENTKIT_H",
        depends_on=("foundation",),
    ),
    FrameworkSpec(
        name="iokit_hid",
        umbrella="IOKit/hid/IOHIDLib.h",
        guard="BEAN_IOHID_H",
        depends_on=("corefoundation",),
        emit_parsed_types=False,
    ),
)

FRAMEWORK_BY_NAME = {spec.name: spec for spec in FRAMEWORKS}

ENUM_PREFIX_ALLOWLIST: dict[str, tuple[str, ...]] = {
    "foundation": ("NS", "UT"),
    "appkit": ("NS", "CI", "CA"),
    "coregraphics": ("CG",),
    "corefoundation": ("CF", "kCF"),
    "quartzcore": ("CA", "kCA"),
    "webkit": ("WK", "NS", "Web"),
    "avfoundation": ("AV", "CM"),
    "coremedia": ("CM", "kCM"),
    "corevideo": ("CV", "kCV"),
    "coredata": ("NS",),
    "coretext": ("CT", "kCT"),
    "imageio": ("kCG", "CG"),
    "metal": ("MTL",),
    "metalkit": ("MTK", "MDL", "MTL"),
    "scenekit": ("SCN", "MDL"),
    "spritekit": ("SK",),
    "gameplaykit": ("GK", "MTL", "MDL"),
    "mapkit": ("MK", "CL"),
    "pdfkit": ("PDF", "NS"),
    "cloudkit": ("CK",),
    "contacts": ("CN",),
    "eventkit": ("EK",),
    "storekit": ("SK",),
    "security": ("errSec", "kSec", "Sec"),
    "carbon": ("k",),
    "iokit": ("kIO", "IO"),
    "iokit_hid": ("kIOHID",),
}

ENUM_NAME_BLOCKLIST_PREFIXES = ("__", "mach_", "dispatch_", "os_", "pthread_")

GENERATION_ORDER: tuple[str, ...] = (
    "corefoundation",
    "coregraphics",
    "coretext",
    "corevideo",
    "coremedia",
    "foundation",
    "security",
    "imageio",
    "iokit",
    "carbon",
    "cloudkit",
    "coredata",
    "contacts",
    "eventkit",
    "metal",
    "quartzcore",
    "appkit",
    "webkit",
    "avfoundation",
    "scenekit",
    "spritekit",
    "metalkit",
    "gameplaykit",
    "mapkit",
    "pdfkit",
    "storekit",
)
