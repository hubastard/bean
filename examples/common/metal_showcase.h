#ifndef BEAN_EXAMPLES_METAL_SHOWCASE_H
#define BEAN_EXAMPLES_METAL_SHOWCASE_H

#include <bean/metal.h>
#include <QuartzCore/CABase.h>
#include <stdio.h>

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
    MTLDevice device;
    MTLCommandQueue queue;
    id pipeline;
    MTLBuffer time_buffer;
    double start_time;
    int ready;
} bean_metal_showcase_state;

static inline BOOL bean_metal_showcase_setup(
    bean_metal_showcase_state* state,
    MTLDevice device,
    MTLPixelFormat pixel_format
) {
    MTLRenderPipelineDescriptor pipeline_desc;
    MTLRenderPipelineColorAttachmentDescriptor color_attachment;
    MTLLibrary library;
    id vertex_fn;
    id fragment_fn;
    NSError error;
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
    state->queue = (MTLCommandQueue)MTLDevice_NewCommandQueue(device);
    if (!state->queue) {
        state->queue = (MTLCommandQueue)MTLDevice_NewCommandQueueWithMaxCommandBufferCount(device, 64);
    }
    if (!state->queue) {
        fprintf(stderr, "Metal showcase: failed to create command queue.\n");
        return NO;
    }

    library = (MTLLibrary)MTLDevice_NewLibraryWithSource_Options_Error(
        device,
        bean_str(bean_metal_showcase_shader_source),
        NULL,
        &error);
    if (!library) {
        const char* message = "unknown error";
        if (error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(error));
        }
        fprintf(stderr, "Metal showcase: shader compile failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    vertex_fn = MTLLibrary_NewFunctionWithName(library, bean_str("bean_vertex"));
    fragment_fn = MTLLibrary_NewFunctionWithName(library, bean_str("bean_fragment"));
    if (!vertex_fn || !fragment_fn) {
        fprintf(stderr, "Metal showcase: failed to load shader functions.\n");
        return NO;
    }

    pipeline_desc = MTLRenderPipelineDescriptor_Alloc();
    pipeline_desc = (MTLRenderPipelineDescriptor)bean_init(pipeline_desc);
    MTLRenderPipelineDescriptor_SetVertexFunction(pipeline_desc, vertex_fn);
    MTLRenderPipelineDescriptor_SetFragmentFunction(pipeline_desc, fragment_fn);
    color_attachment = MTLRenderPipelineColorAttachmentDescriptorArray_ObjectAtIndexedSubscript(
        MTLRenderPipelineDescriptor_ColorAttachments(pipeline_desc),
        0);
    MTLRenderPipelineColorAttachmentDescriptor_SetPixelFormat(color_attachment, pixel_format);

    error = NULL;
    state->pipeline = MTLDevice_NewRenderPipelineStateWithDescriptor_Error(device, pipeline_desc, &error);
    if (!state->pipeline) {
        const char* message = "unknown error";
        if (error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(error));
        }
        fprintf(stderr, "Metal showcase: pipeline creation failed: %s\n", message ? message : "unknown error");
        return NO;
    }

    zero_time = 0.0f;
    state->time_buffer = (MTLBuffer)MTLDevice_NewBufferWithBytes_Length_Options(
        device,
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
    MTLCommandBuffer command_buffer;
    MTLRenderCommandEncoder encoder;
    MTLViewport viewport;
    float time;

    if (!state->ready || !pass || !drawable) {
        return;
    }

    time = (float)(CACurrentMediaTime() - state->start_time);
    {
        void* contents = MTLBuffer_Contents(state->time_buffer);
        if (contents) {
            *(float*)contents = time;
        }
    }

    command_buffer = (MTLCommandBuffer)MTLCommandQueue_CommandBuffer(state->queue);
    encoder = (MTLRenderCommandEncoder)MTLCommandBuffer_RenderCommandEncoderWithDescriptor(
        command_buffer,
        pass);

    viewport.originX = 0.0;
    viewport.originY = 0.0;
    viewport.width = width;
    viewport.height = height;
    viewport.znear = 0.0;
    viewport.zfar = 1.0;

    MTLRenderCommandEncoder_SetViewport(encoder, viewport);
    MTLRenderCommandEncoder_SetRenderPipelineState(encoder, state->pipeline);
    MTLRenderCommandEncoder_SetFragmentBuffer_Offset_AtIndex(
        encoder,
        state->time_buffer,
        0,
        0);
    MTLRenderCommandEncoder_DrawPrimitives_VertexStart_VertexCount(
        encoder,
        MTLPrimitiveTypeTriangle,
        0,
        3);
    MTLCommandEncoder_EndEncoding((MTLCommandEncoder)encoder);
    MTLCommandBuffer_PresentDrawable(command_buffer, drawable);
    MTLCommandBuffer_Commit(command_buffer);
}

#endif /* BEAN_EXAMPLES_METAL_SHOWCASE_H */
