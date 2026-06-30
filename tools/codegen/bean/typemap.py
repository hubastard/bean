"""Map Clang/ObjC types to C types and objc_msgSend casts."""

from __future__ import annotations

import re
from dataclasses import dataclass


@dataclass
class CType:
    spelling: str
    is_struct_return: bool = False
    is_float_return: bool = False


TYPE_REPLACEMENTS = (
    ("instancetype", "id"),
    ("_Nullable", ""),
    ("_Nonnull", ""),
    ("_Null_unspecified", ""),
    ("__kindof ", ""),
    ("NS_SWIFT_NAME", ""),
    ("API_", ""),
)

SKIP_METHOD_PATTERNS: tuple = ()

VARIADIC_FORMAT_SELECTORS = frozenset(
    {
        "stringWithFormat:",
        "initWithFormat:",
        "initWithFormat:locale:",
        "appendFormat:",
        "localizedStringWithFormat:",
    }
)

GL_TYPE_MAP: dict[str, str] = {
    "GLint": "int",
    "GLenum": "unsigned int",
    "GLfloat": "float",
    "GLdouble": "double",
    "GLuint": "unsigned int",
    "GLboolean": "unsigned char",
    "GLsizei": "int",
    "GLbyte": "signed char",
    "GLshort": "short",
    "GLubyte": "unsigned char",
    "GLushort": "unsigned short",
    "GLbitfield": "unsigned int",
    "GLvoid": "void",
}

VOID_TYPEDEF_NAMES = frozenset({"QTMovie", "PDFActionNamedName", "JSValue", "JSContext"})

GLK_TYPE_MAP: dict[str, str] = {
    "_GLKVector2": "vector_float2",
    "_GLKVector3": "vector_float3",
    "_GLKVector4": "vector_float4",
    "_GLKMatrix2": "matrix_float2x2",
    "_GLKMatrix3": "matrix_float3x3",
    "_GLKMatrix4": "matrix_float4x4",
    "GLKVector2": "vector_float2",
    "GLKVector3": "vector_float3",
    "GLKVector4": "vector_float4",
    "GLKMatrix2": "matrix_float2x2",
    "GLKMatrix3": "matrix_float3x3",
    "GLKMatrix4": "matrix_float4x4",
}

KNOWN_STRUCT_TYPES: dict[str, str] = {
    "NSEdgeInsets": "NSEdgeInsets",
    "NSDirectionalEdgeInsets": "NSDirectionalEdgeInsets",
    "NSRange": "NSRange",
    "_NSRange": "NSRange",
    "_NSRect": "NSRect",
    "_NSPoint": "NSPoint",
    "_NSSize": "NSSize",
    "CGRect": "CGRect",
    "CGPoint": "CGPoint",
    "CGSize": "CGSize",
    "CGVector": "CGVector",
    "AudioComponentDescription": "AudioComponentDescription",
    "CLLocationCoordinate2D": "CLLocationCoordinate2D",
    "MKCoordinateSpan": "MKCoordinateSpan",
    "MKMapSize": "MKMapSize",
    "SCNVector3": "SCNVector3",
    "SCNVector4": "SCNVector4",
    "CAFrameRateRange": "CAFrameRateRange",
    "CGAffineTransform": "CGAffineTransform",
    "CATransform3D": "CATransform3D",
    "CVTimeStamp": "CVTimeStamp",
    "MTL4BufferRange": "MTL4BufferRange",
    "NSFastEnumerationState": "NSFastEnumerationState",
    "GKBox": "GKBox",
    "GKQuad": "GKQuad",
    "GKTriangle": "GKTriangle",
    "AudioTimeStamp": "AudioTimeStamp",
    "AVAudio3DPoint": "AVAudio3DPoint",
    "AVAudio3DVector": "AVAudio3DVector",
    "AVAudio3DVectorOrientation": "AVAudio3DVectorOrientation",
    "AVAudio3DAngularOrientation": "AVAudio3DAngularOrientation",
    "AVAudioConverterPrimeInfo": "AVAudioConverterPrimeInfo",
}

# Clang reports many CF/CG/CoreMedia opaque types as `struct Foo*`.
OPAQUE_POINTER_TYPES: dict[str, str] = {
    "CGColor": "CGColorRef",
    "CGColorSpace": "CGColorSpaceRef",
    "CGImage": "CGImageRef",
    "CGContext": "CGContextRef",
    "CGImageSource": "CGImageSourceRef",
    "CGImageDestination": "CGImageDestinationRef",
    "CGPath": "CGPathRef",
    "CGPattern": "CGPatternRef",
    "CGGradient": "CGGradientRef",
    "CGFont": "CGFontRef",
    "CGLayer": "CGLayerRef",
    "CGDataProvider": "CGDataProviderRef",
    "CGDataConsumer": "CGDataConsumerRef",
    "CGFunction": "CGFunctionRef",
    "CGShading": "CGShadingRef",
    "CGEvent": "CGEventRef",
    "CGEventSource": "CGEventSourceRef",
    "CGEventTapProxy": "CGEventTapProxy",
    "CGDisplayStream": "CGDisplayStreamRef",
    "CGDisplayStreamFrameAvailableHandler": "void*",
    "__CVBuffer": "CVBufferRef",
    "__IOSurface": "IOSurfaceRef",
    "__CFRunLoop": "CFRunLoopRef",
    "__CFString": "CFStringRef",
    "__SecIdentity": "SecIdentityRef",
    "__SecTrust": "SecTrustRef",
    "__SecCertificate": "SecCertificateRef",
    "__SecKey": "SecKeyRef",
    "__CGEvent": "CGEventRef",
    "_NSZone": "NSZone*",
    "opaqueCMSampleBuffer": "CMSampleBufferRef",
    "OpaqueCMClock": "CMClockRef",
    "OpaqueCMTimebase": "CMTimebaseRef",
    "OpaqueCMBlockBuffer": "CMBlockBufferRef",
    "OpaqueCMSampleBuffer": "CMSampleBufferRef",
    "OpaqueIconRef": "void*",
    "_CGLContextObject": "void*",
    "_CGLPixelFormatObject": "void*",
    "_CGLPBufferObject": "void*",
    "_NSModalSession": "void*",
    "__NSAppleEventManagerSuspension": "void*",
    "__CFDictionary": "CFDictionaryRef",
    "OpaqueCMTaggedBufferGroup": "CMTaggedBufferGroupRef",
    "OpaqueAudioComponent": "void*",
    "OpaqueMusicSequence": "void*",
    "OpaqueJSValue": "void*",
    "OpaqueJSContext": "void*",
    "ComponentInstanceRecord": "void*",
    "CGPDFPage": "void*",
}

UNSUPPORTED_TYPE_NAMES: frozenset[str] = frozenset()


def sanitize_qual_type(qual_type: str) -> str:
    text = qual_type.strip()
    for old, new in TYPE_REPLACEMENTS:
        text = text.replace(old, new)
    if "__attribute__((ext_vector_type" in text:
        return "void*"
    text = re.sub(r"^struct__", "struct __", text)
    text = re.sub(r"\s+", " ", text).strip()
    # Strip Objective-C lightweight generics: NSArray<Foo *>* -> NSArray *
    text = re.sub(r"([A-Za-z_][A-Za-z0-9_]*)<[^>]+>", r"\1", text)
    text = text.replace("ObjectTypeconst", "const id").replace("ObjectType", "id")
    text = text.replace("KeyType", "id").replace("ValueType", "id")
    if text == "K" or text == "V":
        text = "id*"
    text = text.replace("idconst*", "const id*").replace("idconst", "const id")
    text = text.replace("unsignedlonglong", "unsigned long long")
    text = re.sub(r",id\b", "*", text)
    text = re.sub(r"\s*>\s*\*", "*", text)
    text = re.sub(r" \*>\*", "*", text)
    text = re.sub(r"\*>\s*\*", "*", text)
    text = re.sub(r"\*>\*", "*", text)
    text = re.sub(r"\*>\s*", "*", text)
    text = re.sub(r">\s*\*", "*", text)
    text = re.sub(r"\*\s*\*", "*", text)
    text = re.sub(r"([A-Za-z_][A-Za-z0-9_]*)>\*", r"\1*", text)
    text = re.sub(r"([A-Za-z_][A-Za-z0-9_]*)>\s*\*", r"\1*", text)
    text = re.sub(r"\s+", " ", text).strip()
    return text


def _resolve_struct_type(struct_name: str) -> CType | None:
    mapped = KNOWN_STRUCT_TYPES.get(struct_name)
    if mapped is None:
        return None
    return CType(mapped, is_struct_return=True)


def _resolve_opaque_pointer(struct_name: str) -> str | None:
    mapped = OPAQUE_POINTER_TYPES.get(struct_name)
    if mapped is not None:
        return mapped
    if struct_name.startswith("__"):
        return "void*"
    return None


def qual_type_to_c(qual_type: str) -> CType:
    text = sanitize_qual_type(qual_type)
    if not text or text == "void":
        return CType("void")
    clean = text.replace("const ", "").strip()
    if clean in GL_TYPE_MAP:
        mapped = GL_TYPE_MAP[clean]
        if text.startswith("const "):
            return CType(f"const {mapped}")
        return CType(mapped)
    if clean in VOID_TYPEDEF_NAMES:
        return CType("void*")
    if clean in GLK_TYPE_MAP:
        return CType(GLK_TYPE_MAP[clean], is_struct_return=True)
    if clean.startswith("struct _GLK") or clean.startswith("struct GLK"):
        inner = clean[7:].strip() if clean.startswith("struct ") else clean
        if inner in GLK_TYPE_MAP:
            return CType(GLK_TYPE_MAP[inner], is_struct_return=True)
    if clean in ("CGLineCap", "CGLineJoin"):
        return CType(clean)
    if text.startswith("struct "):
        remainder = text[len("struct ") :].strip()
        if remainder.endswith("*"):
            struct_name = remainder[:-1].strip()
            opaque = _resolve_opaque_pointer(struct_name)
            if opaque is not None:
                return CType(opaque)
            mapped = KNOWN_STRUCT_TYPES.get(struct_name)
            if mapped is not None:
                return CType(f"{mapped}*")
            return CType(f"struct {struct_name}*")
        known = _resolve_struct_type(remainder)
        if known is not None:
            return known
        if remainder in GLK_TYPE_MAP:
            return CType(GLK_TYPE_MAP[remainder], is_struct_return=True)
        return CType(f"struct {remainder}", is_struct_return=True)
    if text == "id":
        return CType("id")
    if text == "Class":
        return CType("bean_class")
    if text == "SEL":
        return CType("SEL")
    if text in ("BOOL", "bool", "_Bool"):
        return CType("BOOL")
    if text in ("NSInteger", "long", "signed long"):
        return CType("NSInteger")
    if text in ("NSUInteger", "unsigned long", "unsigned int"):
        return CType("NSUInteger")
    if text in ("CGFloat", "float", "double"):
        return CType("CGFloat", is_float_return=text in ("CGFloat", "float", "double"))
    if text in ("NSTimeInterval",):
        return CType("NSTimeInterval")
    if text in ("NSRect",):
        return CType("NSRect", is_struct_return=True)
    if text in ("NSPoint",):
        return CType("NSPoint", is_struct_return=True)
    if text in ("NSSize",):
        return CType("NSSize", is_struct_return=True)
    if text in ("NSRange",):
        return CType("NSRange", is_struct_return=True)
    if text in ("CGRect",):
        return CType("CGRect", is_struct_return=True)
    if text in ("CGPoint",):
        return CType("CGPoint", is_struct_return=True)
    if text in ("CGVector",):
        return CType("CGVector", is_struct_return=True)
    if text in ("CGAffineTransform",):
        return CType("CGAffineTransform", is_struct_return=True)
    if text in ("AudioComponentDescription",):
        return CType("AudioComponentDescription", is_struct_return=True)
    if text in ("CVTimeStamp",):
        return CType("CVTimeStamp", is_struct_return=True)
    if text in (
        "MTL4BufferRange",
        "NSFastEnumerationState",
        "GKBox",
        "GKQuad",
        "GKTriangle",
        "AudioTimeStamp",
        "AVAudio3DPoint",
        "AVAudio3DVector",
        "AVAudio3DVectorOrientation",
        "AVAudio3DAngularOrientation",
        "AVAudioConverterPrimeInfo",
    ):
        return CType(text, is_struct_return=True)
    if text in ("CGSize",):
        return CType("CGSize", is_struct_return=True)
    if text in (
        "CGColorRef", "CGContextRef", "CGImageRef", "CGColorSpaceRef", "CGImageSourceRef",
        "CGLayerRef", "CGPathRef", "CGEventRef", "CFStringRef", "CFTypeRef", "CFRunLoopRef",
        "CVBufferRef", "IOSurfaceRef", "CMSampleBufferRef", "CMClockRef", "CMTimebaseRef",
        "SecIdentityRef", "SecTrustRef", "SecCertificateRef", "SecKeyRef",
    ):
        return CType(text)
    if text.endswith("*"):
        base = text[:-1].strip()
        if base.startswith("const "):
            inner = base[6:].strip()
            if inner and inner[0].isupper() and " " not in inner:
                return CType(f"const {inner}")
            return CType(f"const {inner}*")
        if base.endswith(" const"):
            inner = base[:-6].strip()
            return CType(f"const {inner}*")
        if " " in base:
            return CType(f"{base}*")
        if base in VOID_TYPEDEF_NAMES:
            return CType("void*")
        if base == "NSZone":
            return CType("NSZone*")
        if base in ("K", "V", "id"):
            return CType("id*")
        if base in KNOWN_STRUCT_TYPES:
            return CType(f"{KNOWN_STRUCT_TYPES[base]}*")
        if re.match(r"^DOM[A-Z]", base):
            return CType("void*")
        if base and base[0].isupper():
            return CType(base)
        return CType(f"{base}*")
    if text.endswith("^)"):
        return CType(text)
    if "(^" in text:
        return CType("void*")
    if "(*" in text:
        return CType("void*")
    if text.startswith("void (^") or text.startswith("id (^"):
        return CType("void*")
    if re.match(r"^DOM[A-Z]", text.rstrip("*").strip()):
        return CType("void*")
    if text.startswith("enum "):
        return CType(text.replace("enum ", "", 1))
    if text.startswith("union "):
        inner = text.replace("union ", "", 1).strip()
        if inner in GLK_TYPE_MAP:
            return CType(GLK_TYPE_MAP[inner], is_struct_return=True)
        return CType(inner)
    if text.startswith("const "):
        return qual_type_to_c(text[6:])
    if text == "NSZone":
        return CType("NSZone*")
    if text in ("K", "V"):
        return CType("id*")
    if text in GLK_TYPE_MAP:
        return CType(GLK_TYPE_MAP[text], is_struct_return=True)
    if text.startswith("__"):
        return CType("void*")
    return CType(text)


def selector_to_c_suffix(selector: str) -> str:
    if not selector:
        return "invoke"
    parts = selector.rstrip(":").split(":")
    out = []
    for part in parts:
        if not part:
            continue
        out.append(part[0].upper() + part[1:] if part else part)
    if selector.endswith(":"):
        return "_".join(out) + "_"
    return "_".join(out)


def c_function_name(class_name: str, selector: str, is_class_method: bool) -> str:
    suffix = selector_to_c_suffix(selector)
    if suffix.endswith("_"):
        suffix = suffix[:-1]
    return f"{class_name}_{suffix}"


def _type_is_valid(spelling: str) -> bool:
    if not spelling or spelling == "void":
        return True
    clean = spelling.replace("const ", "").strip()
    if clean in UNSUPPORTED_TYPE_NAMES:
        return False
    if "," in spelling and "(*" not in spelling:
        return False
    if clean.startswith("simd_"):
        return True
    if clean.startswith("vector_") or clean.startswith("matrix_"):
        return True
    if clean in GLK_TYPE_MAP.values():
        return True
    if clean in ("CGLineCap", "CGLineJoin"):
        return True
    if clean.startswith("DOM"):
        return False
    if clean.startswith("JS") and clean not in {"JSON"}:
        return False
    if clean.startswith("GL") and clean not in GL_TYPE_MAP:
        if re.match(r"^GL[A-Za-z0-9_]+$", clean):
            return False
    if clean.startswith("CGL") and clean not in {"CGLayerRef"}:
        return False
    if clean.startswith("struct _CGL"):
        return False
    if clean == "void*":
        return True
    if "(*" in spelling:
        return False
    if ">*" in spelling or "*>" in spelling or " *>*" in spelling:
        return False
    if spelling in {"KeyType", "ObjectType"}:
        return False
    return True


def is_unsupported_method(selector: str, params: list[CType], return_type: CType | None = None) -> str | None:
    known_structs = set(KNOWN_STRUCT_TYPES.keys()) | set(KNOWN_STRUCT_TYPES.values())

    def struct_is_unknown(spelling: str) -> bool:
        if not spelling.startswith("struct "):
            return False
        struct_name = spelling[7:].strip().rstrip("*").strip()
        return struct_name not in known_structs

    if return_type is not None and struct_is_unknown(return_type.spelling):
        return "incomplete struct return type"
    if selector not in VARIADIC_FORMAT_SELECTORS:
        for pattern in SKIP_METHOD_PATTERNS:
            if pattern.search(selector):
                return "variadic"
    types = list(params)
    if return_type is not None:
        types.append(return_type)
    for param in types:
        if struct_is_unknown(param.spelling):
            return "incomplete struct parameter type"
        if not _type_is_valid(param.spelling):
            return "invalid generated type"
        if "opaque" in param.spelling and param.spelling.startswith("struct "):
            return "opaque internal type"
        if "(*" in param.spelling:
            return "function pointer parameter"
        if "__" in param.spelling and param.spelling != "void*":
            return "internal type"
    return None


def normalize_return_type_spelling(spelling: str) -> str:
    text = spelling.strip()
    while text.startswith("const "):
        text = text[6:].strip()
    return text


def msg_send_name(return_type: CType) -> str:
    if return_type.is_struct_return:
        return "bean_msgSend_stret"
    if return_type.is_float_return:
        return "bean_msgSend_fpret"
    return "bean_msgSend"


def build_msg_send_cast(
    return_type: CType,
    receiver_type: str,
    params: list[CType],
) -> str:
    send = msg_send_name(return_type)
    arg_types = [receiver_type, "SEL"] + [p.spelling for p in params]
    ret = normalize_return_type_spelling(return_type.spelling)
    signature = f"{ret} (*)({', '.join(arg_types)})"
    return f"(({signature}){send})"
