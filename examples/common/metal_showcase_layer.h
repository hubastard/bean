#ifndef BEAN_EXAMPLES_METAL_SHOWCASE_LAYER_H
#define BEAN_EXAMPLES_METAL_SHOWCASE_LAYER_H

#include "metal_showcase.h"
#include <CoreFoundation/CoreFoundation.h>
#include <bean/appkit.h>
#include <bean/quartzcore.h>

static bean_metal_showcase_state g_bean_metal_layer_state;
static CAMetalLayer g_bean_metal_layer = NULL;
static NSView g_bean_metal_host_view = NULL;
static NSWindow g_bean_metal_host_window = NULL;
static id g_bean_metal_layer_target = NULL;
static CADisplayLink g_bean_metal_display_link = NULL;

static inline void bean_metal_layer_sync_layout(void) {
    CGRect bounds;
    CGFloat scale;
    CGSize drawable_size;

    if (!g_bean_metal_layer || !g_bean_metal_host_view) {
        return;
    }

    bounds = NSView_Bounds(g_bean_metal_host_view);
    scale = 2.0;
    if (g_bean_metal_host_window) {
        scale = NSWindow_BackingScaleFactor(g_bean_metal_host_window);
    }

    CALayer_SetFrame((CALayer)g_bean_metal_layer, bounds);
    CALayer_SetContentsScale((CALayer)g_bean_metal_layer, scale);

    drawable_size.width = bounds.size.width * scale;
    drawable_size.height = bounds.size.height * scale;
    if (drawable_size.width < 1.0 || drawable_size.height < 1.0) {
        return;
    }

    CAMetalLayer_SetDrawableSize(g_bean_metal_layer, drawable_size);
}

static void bean_metal_layer_tick(id self, SEL _cmd, CADisplayLink link) {
    MTLRenderPassDescriptor pass;
    MTLRenderPassColorAttachmentDescriptor color_attachment;
    MTLClearColor clear_color;
    id drawable;
    id texture;
    CGSize drawable_size;

    (void)self;
    (void)_cmd;
    (void)link;

    if (!g_bean_metal_layer) {
        return;
    }

    bean_metal_layer_sync_layout();

    if (!g_bean_metal_layer_state.ready) {
        if (!bean_metal_showcase_setup(
                &g_bean_metal_layer_state,
                CAMetalLayer_Device(g_bean_metal_layer),
                CAMetalLayer_PixelFormat(g_bean_metal_layer))) {
            return;
        }
    }

    drawable = CAMetalLayer_NextDrawable(g_bean_metal_layer);
    if (!drawable) {
        return;
    }

    texture = ((id (*)(id, SEL))bean_msgSend)(drawable, bean_sel("texture"));
    pass = MTLRenderPassDescriptor_RenderPassDescriptor();
    color_attachment = MTLRenderPassColorAttachmentDescriptorArray_ObjectAtIndexedSubscript(
        MTLRenderPassDescriptor_ColorAttachments(pass),
        0);
    MTLRenderPassAttachmentDescriptor_SetTexture(
        (MTLRenderPassAttachmentDescriptor)color_attachment,
        texture);
    MTLRenderPassAttachmentDescriptor_SetLoadAction(
        (MTLRenderPassAttachmentDescriptor)color_attachment,
        MTLLoadActionClear);
    MTLRenderPassAttachmentDescriptor_SetStoreAction(
        (MTLRenderPassAttachmentDescriptor)color_attachment,
        MTLStoreActionStore);
    clear_color = (MTLClearColor){ 0.05, 0.05, 0.08, 1.0 };
    MTLRenderPassColorAttachmentDescriptor_SetClearColor(color_attachment, clear_color);

    drawable_size = CAMetalLayer_DrawableSize(g_bean_metal_layer);
    bean_metal_showcase_draw(
        &g_bean_metal_layer_state,
        pass,
        drawable_size.width,
        drawable_size.height,
        drawable);
}

static inline void bean_metal_showcase_start_metal_layer_animation(void) {
    if (g_bean_metal_display_link || !g_bean_metal_layer_target) {
        return;
    }

    g_bean_metal_display_link = CADisplayLink_DisplayLinkWithTarget_Selector(
        g_bean_metal_layer_target,
        bean_sel("tick:"));
    CADisplayLink_SetPaused(g_bean_metal_display_link, NO);
    CADisplayLink_AddToRunLoop_ForMode(
        g_bean_metal_display_link,
        NSRunLoop_MainRunLoop(),
        (NSString)(void*)kCFRunLoopCommonModes);
}

static inline BOOL bean_metal_showcase_attach_metal_layer(NSView host_view, NSWindow window) {
    id device;
    CAMetalLayer layer;
    static const bean_method_spec methods[] = {
        { "tick:", (IMP)bean_metal_layer_tick, "v@:@" },
    };
    static bean_class tick_cls = NULL;

    device = MTLCreateSystemDefaultDevice();
    if (!device) {
        fprintf(stderr, "Metal showcase: no default Metal device available.\n");
        return NO;
    }

    layer = (CAMetalLayer)((id (*)(Class, SEL))objc_msgSend)(
        (Class)bean_cls("CAMetalLayer"),
        bean_sel("layer"));
    CAMetalLayer_SetDevice(layer, device);
    CAMetalLayer_SetPixelFormat(layer, MTLPixelFormatBGRA8Unorm);
    CAMetalLayer_SetFramebufferOnly(layer, YES);

    NSView_SetWantsLayer(host_view, YES);
    NSView_SetLayer(host_view, (CALayer)layer);

    g_bean_metal_layer = layer;
    g_bean_metal_host_view = host_view;
    g_bean_metal_host_window = window;

    if (!tick_cls) {
        tick_cls = bean_make_subclass(
            "BeanMetalShowcaseLayerDriver",
            bean_cls("NSObject"),
            methods,
            sizeof(methods) / sizeof(methods[0]),
            0);
        if (!tick_cls) {
            fprintf(stderr, "Metal showcase: failed to create display-link target.\n");
            return NO;
        }
    }

    g_bean_metal_layer_target = bean_init((id)bean_alloc(tick_cls));
    bean_metal_layer_sync_layout();
    return YES;
}

#endif /* BEAN_EXAMPLES_METAL_SHOWCASE_LAYER_H */
