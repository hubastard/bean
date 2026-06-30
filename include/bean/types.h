#ifndef BEAN_TYPES_H
#define BEAN_TYPES_H

#include <stddef.h>
#include <stdint.h>

#include <CoreFoundation/CoreFoundation.h>
#include <CoreGraphics/CoreGraphics.h>
#include <simd/simd.h>

struct objc_object;
struct objc_class;
struct objc_selector;

typedef struct objc_object* bean_id;
typedef struct objc_class* bean_class;
typedef const struct objc_selector* bean_sel;

#if __LP64__
typedef long NSInteger;
typedef unsigned long NSUInteger;
#else
typedef int NSInteger;
typedef unsigned int NSUInteger;
#endif

typedef CGFloat NSFloat;
typedef struct _NSPoint {
    NSFloat x;
    NSFloat y;
} NSPoint;

typedef struct _NSSize {
    NSFloat width;
    NSFloat height;
} NSSize;

typedef struct _NSRect {
    NSPoint origin;
    NSSize size;
} NSRect;

typedef struct _NSRange {
    NSUInteger location;
    NSUInteger length;
} NSRange;

typedef struct NSEdgeInsets {
    NSFloat top;
    NSFloat left;
    NSFloat bottom;
    NSFloat right;
} NSEdgeInsets;

typedef struct NSDirectionalEdgeInsets {
    NSFloat top;
    NSFloat leading;
    NSFloat bottom;
    NSFloat trailing;
} NSDirectionalEdgeInsets;

typedef struct CLLocationCoordinate2D {
    double latitude;
    double longitude;
} CLLocationCoordinate2D;

typedef struct MKCoordinateSpan {
    double latitudeDelta;
    double longitudeDelta;
} MKCoordinateSpan;

typedef struct MKMapSize {
    double width;
    double height;
} MKMapSize;

typedef struct SCNVector3 {
    NSFloat x;
    NSFloat y;
    NSFloat z;
} SCNVector3;

typedef struct SCNVector4 {
    NSFloat x;
    NSFloat y;
    NSFloat z;
    NSFloat w;
} SCNVector4;

typedef struct CAFrameRateRange {
    float minimum;
    float maximum;
    float preferred;
} CAFrameRateRange;

typedef struct CATransform3D {
    NSFloat m11, m12, m13, m14;
    NSFloat m21, m22, m23, m24;
    NSFloat m31, m32, m33, m34;
    NSFloat m41, m42, m43, m44;
} CATransform3D;

typedef uint64_t MTLGPUAddress;

typedef struct MTL4BufferRange {
    MTLGPUAddress bufferAddress;
    uint64_t length;
} MTL4BufferRange;

typedef struct {
    unsigned long state;
    bean_id *itemsPtr;
    unsigned long *mutationsPtr;
    unsigned long extra[5];
} NSFastEnumerationState;

typedef struct GKBox {
    vector_float3 boxMin;
    vector_float3 boxMax;
} GKBox;

typedef struct GKQuad {
    vector_float2 quadMin;
    vector_float2 quadMax;
} GKQuad;

typedef struct GKTriangle {
    vector_float3 points[3];
} GKTriangle;

typedef uint32_t AVAudioFrameCount;

typedef struct AVAudio3DPoint {
    float x;
    float y;
    float z;
} AVAudio3DPoint;

typedef AVAudio3DPoint AVAudio3DVector;

typedef struct AVAudio3DVectorOrientation {
    AVAudio3DVector forward;
    AVAudio3DVector up;
} AVAudio3DVectorOrientation;

typedef struct AVAudio3DAngularOrientation {
    float yaw;
    float pitch;
    float roll;
} AVAudio3DAngularOrientation;

typedef struct AVAudioConverterPrimeInfo {
    AVAudioFrameCount leadingFrames;
    AVAudioFrameCount trailingFrames;
} AVAudioConverterPrimeInfo;

typedef int32_t CGLineCap;
typedef int32_t CGLineJoin;

typedef double NSTimeInterval;

typedef struct _NSZone NSZone;

typedef unsigned short unichar;

static inline NSPoint NSMakePoint(NSFloat x, NSFloat y) {
    NSPoint p = {x, y};
    return p;
}

static inline NSSize NSMakeSize(NSFloat width, NSFloat height) {
    NSSize s = {width, height};
    return s;
}

static inline NSRect NSMakeRect(NSFloat x, NSFloat y, NSFloat width, NSFloat height) {
    NSRect r = {{x, y}, {width, height}};
    return r;
}

static inline NSRange NSMakeRange(NSUInteger loc, NSUInteger len) {
    NSRange r = {loc, len};
    return r;
}

#endif /* BEAN_TYPES_H */
