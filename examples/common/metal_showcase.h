#ifndef BEAN_EXAMPLES_METAL_SHOWCASE_H
#define BEAN_EXAMPLES_METAL_SHOWCASE_H

#include <bean/metal.h>
#include <QuartzCore/CABase.h>
#include <stdio.h>

typedef struct bean_mtl_viewport {
    double originX;
    double originY;
    double width;
    double height;
    double znear;
    double zfar;
} bean_mtl_viewport;

#ifdef __cplusplus
extern "C" {
#endif

id MTLCreateSystemDefaultDevice(void);

#ifdef __cplusplus
}
#endif

static const char* const bean_metal_showcase_shader_source =
    "#include <metal_stdlib>\n"
    "using namespace metal;\n"
    "struct VertexOut {\n"
    "    float4 position [[position]];\n"
    "    float2 uv;\n"
    "};\n"
    "vertex VertexOut bean_vertex(uint vid [[vertex_id]]) {\n"
    "    float2 positions[3] = { float2(-1.0, -1.0), float2(3.0, -1.0), float2(-1.0, 3.0) };\n"
    "    VertexOut out;\n"
    "    out.position = float4(positions[vid], 0.0, 1.0);\n"
    "    out.uv = positions[vid] * 0.5 + 0.5;\n"
    "    return out;\n"
    "}\n"
    "fragment float4 bean_fragment(\n"
    "    VertexOut in [[stage_in]],\n"
    "    constant float& time [[buffer(0)]]\n"
    ") {\n"
    "    float t = time;\n"
    "    float3 color = float3(\n"
    "        0.5 + 0.5 * sin(t + in.uv.x * 6.28318),\n"
    "        0.5 + 0.5 * sin(t * 1.31 + in.uv.y * 6.28318),\n"
    "        0.5 + 0.5 * sin(t * 0.73 + in.uv.x * in.uv.y * 6.28318));\n"
    "    return float4(color, 1.0);\n"
    "}\n";

typedef struct bean_metal_showcase_state {
    id device;
    id queue;
    id pipeline;
    id time_buffer;
    double start_time;
    int ready;
} bean_metal_showcase_state;

static inline id bean_metal_device_new_library(id device, NSError** error) {
    return ((id (*)(id, SEL, NSString, id, NSError**))bean_msgSend)(
        device,
        bean_sel("newLibraryWithSource:options:error:"),
        NSString_StringWithUTF8String(bean_metal_showcase_shader_source),
        NULL,
        error);
}

static inline id bean_metal_device_new_pipeline(
    id device,
    MTLRenderPipelineDescriptor descriptor,
    NSError** error
) {
    return ((id (*)(id, SEL, MTLRenderPipelineDescriptor, NSError**))bean_msgSend)(
        device,
        bean_sel("newRenderPipelineStateWithDescriptor:error:"),
        descriptor,
        error);
}

static inline BOOL bean_metal_showcase_setup(
    bean_metal_showcase_state* state,
    id device,
    MTLPixelFormat pixel_format
) {
    MTLRenderPipelineDescriptor pipeline_desc;
    MTLRenderPipelineColorAttachmentDescriptor color_attachment;
    id library;
    id vertex_fn;
    id fragment_fn;
    NSError* error;
    float zero_time;

    if (state->ready) {
        return YES;
    }

    if (!device) {
        fprintf(stderr, "Metal showcase: Metal device is nil.\n");
        return NO;
    }

    error = NULL;
    state->device = device;
    state->queue = ((id (*)(id, SEL))bean_msgSend)(device, bean_sel("newCommandQueue"));
    if (!state->queue) {
        state->queue = ((id (*)(id, SEL, NSUInteger))bean_msgSend)(
            device,
            bean_sel("newCommandQueueWithMaxCommandBufferCount:"),
            64);
    }
    if (!state->queue) {
        fprintf(stderr, "Metal showcase: failed to create command queue.\n");
        return NO;
    }

    library = bean_metal_device_new_library(device, &error);
    if (!library) {
        const char* message = "unknown error";
        if (error && *error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(*error));
        }
        fprintf(stderr, "Metal showcase: shader compile failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    vertex_fn = ((id (*)(id, SEL, NSString))bean_msgSend)(
        library, bean_sel("newFunctionWithName:"), NSString_StringWithUTF8String("bean_vertex"));
    fragment_fn = ((id (*)(id, SEL, NSString))bean_msgSend)(
        library, bean_sel("newFunctionWithName:"), NSString_StringWithUTF8String("bean_fragment"));
    if (!vertex_fn || !fragment_fn) {
        fprintf(stderr, "Metal showcase: failed to load shader functions.\n");
        return NO;
    }

    pipeline_desc = (MTLRenderPipelineDescriptor)bean_alloc(bean_cls("MTLRenderPipelineDescriptor"));
    pipeline_desc = (MTLRenderPipelineDescriptor)bean_init(pipeline_desc);
    MTLRenderPipelineDescriptor_SetVertexFunction(pipeline_desc, vertex_fn);
    MTLRenderPipelineDescriptor_SetFragmentFunction(pipeline_desc, fragment_fn);
    color_attachment = MTLRenderPipelineColorAttachmentDescriptorArray_ObjectAtIndexedSubscript(
        MTLRenderPipelineDescriptor_ColorAttachments(pipeline_desc),
        0);
    MTLRenderPipelineColorAttachmentDescriptor_SetPixelFormat(color_attachment, pixel_format);

    error = NULL;
    state->pipeline = bean_metal_device_new_pipeline(device, pipeline_desc, &error);
    if (!state->pipeline) {
        const char* message = "unknown error";
        if (error && *error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(*error));
        }
        fprintf(stderr, "Metal showcase: pipeline creation failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    zero_time = 0.0f;
    state->time_buffer = ((id (*)(id, SEL, const void*, NSUInteger, MTLResourceOptions))bean_msgSend)(
        device,
        bean_sel("newBufferWithBytes:length:options:"),
        &zero_time,
        sizeof(zero_time),
        MTLResourceStorageModeShared);
    state->start_time = CACurrentMediaTime();
    state->ready = 1;
    return YES;
}

static inline void bean_metal_showcase_draw(
    bean_metal_showcase_state* state,
    MTLRenderPassDescriptor pass,
    double width,
    double height,
    id drawable
) {
    id command_buffer;
    id encoder;
    bean_mtl_viewport viewport;
    float time;

    if (!state->ready || !pass || !drawable) {
        return;
    }

    time = (float)(CACurrentMediaTime() - state->start_time);
    {
        void* contents = ((void* (*)(id, SEL))bean_msgSend)(state->time_buffer, bean_sel("contents"));
        if (contents) {
            *(float*)contents = time;
        }
    }

    command_buffer = ((id (*)(id, SEL))bean_msgSend)(state->queue, bean_sel("commandBuffer"));
    encoder = ((id (*)(id, SEL, MTLRenderPassDescriptor))bean_msgSend)(
        command_buffer,
        bean_sel("renderCommandEncoderWithDescriptor:"),
        pass);

    viewport.originX = 0.0;
    viewport.originY = 0.0;
    viewport.width = width;
    viewport.height = height;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;

    ((void (*)(id, SEL, bean_mtl_viewport))bean_msgSend)(encoder, bean_sel("setViewport:"), viewport);
    ((void (*)(id, SEL, id))bean_msgSend)(encoder, bean_sel("setRenderPipelineState:"), state->pipeline);
    ((void (*)(id, SEL, id, NSUInteger, NSUInteger))bean_msgSend)(
        encoder,
        bean_sel("setFragmentBuffer:offset:atIndex:"),
        state->time_buffer,
        0,
        0);
    ((void (*)(id, SEL, MTLPrimitiveType, NSUInteger, NSUInteger))bean_msgSend)(
        encoder,
        bean_sel("drawPrimitives:vertexStart:vertexCount:"),
        MTLPrimitiveTypeTriangle,
        0,
        3);
    ((void (*)(id, SEL))bean_msgSend)(encoder, bean_sel("endEncoding"));
    ((void (*)(id, SEL, id))bean_msgSend)(command_buffer, bean_sel("presentDrawable:"), drawable);
    ((void (*)(id, SEL))bean_msgSend)(command_buffer, bean_sel("commit"));
}

#endif /* BEAN_EXAMPLES_METAL_SHOWCASE_H */
