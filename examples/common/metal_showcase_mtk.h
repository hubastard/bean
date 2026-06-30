#ifndef BEAN_EXAMPLES_METAL_SHOWCASE_MTK_H
#define BEAN_EXAMPLES_METAL_SHOWCASE_MTK_H

#include "metal_showcase.h"
#include <bean/metalkit.h>

static bean_metal_showcase_state g_bean_metal_mtk_state;

static void bean_metal_mtk_draw(id self, SEL _cmd, MTKView view) {
    MTLRenderPassDescriptor pass;
    id drawable;
    CGSize size;

    (void)self;
    (void)_cmd;

    if (!g_bean_metal_mtk_state.ready) {
        id device = MTKView_Device(view);
        if (!device) {
            device = MTLCreateSystemDefaultDevice();
            if (device) {
                MTKView_SetDevice(view, device);
            }
        }
        if (!bean_metal_showcase_setup(
                &g_bean_metal_mtk_state,
                device,
                MTKView_ColorPixelFormat(view))) {
            return;
        }
    }

    pass = MTKView_CurrentRenderPassDescriptor(view);
    drawable = MTKView_CurrentDrawable(view);
    if (!pass || !drawable) {
        return;
    }

    size = MTKView_DrawableSize(view);
    bean_metal_showcase_draw(
        &g_bean_metal_mtk_state,
        pass,
        size.width,
        size.height,
        drawable);
}

static void bean_metal_mtk_resize(id self, SEL _cmd, MTKView view, CGSize size) {
    (void)self;
    (void)_cmd;
    (void)view;
    (void)size;
}

static inline id bean_metal_showcase_mtk_delegate(void) {
    static id delegate = NULL;

    if (delegate) {
        return delegate;
    }

    static const bean_method_spec methods[] = {
        { "drawInMTKView:", (IMP)bean_metal_mtk_draw, "v@:@" },
        { "mtkView:drawableSizeWillChange:", (IMP)bean_metal_mtk_resize, "v@:@{CGSize=dd}" },
    };

    bean_class cls = bean_make_subclass(
        "BeanMetalShowcaseMTKDelegate",
        bean_cls("NSObject"),
        methods,
        sizeof(methods) / sizeof(methods[0]),
        0);
    if (!cls) {
        return NULL;
    }

    delegate = bean_init((id)bean_alloc(cls));
    return delegate;
}

static inline void bean_metal_showcase_configure_mtk_view(MTKView view) {
    id device;
    id delegate;

    device = MTLCreateSystemDefaultDevice();
    if (!device) {
        fprintf(stderr, "Metal showcase: no default Metal device available.\n");
        return;
    }

    delegate = bean_metal_showcase_mtk_delegate();
    if (!delegate) {
        fprintf(stderr, "Metal showcase: failed to create MTKView delegate.\n");
        return;
    }

    MTKView_SetDevice(view, device);
    NSView_SetAutoresizingMask((NSView)view, NSViewWidthSizable | NSViewHeightSizable);
    MTKView_SetDelegate(view, delegate);
    MTKView_SetPaused(view, NO);
    MTKView_SetEnableSetNeedsDisplay(view, NO);
    MTKView_SetAutoResizeDrawable(view, YES);
    MTKView_SetColorPixelFormat(view, MTLPixelFormatBGRA8Unorm);
    MTKView_SetClearColor(
        view,
        (MTLClearColor){ 0.05, 0.05, 0.08, 1.0 });

    if (!bean_metal_showcase_setup(
            &g_bean_metal_mtk_state,
            device,
            MTKView_ColorPixelFormat(view))) {
        fprintf(stderr, "Metal showcase: MTKView setup failed.\n");
    }
}

#endif /* BEAN_EXAMPLES_METAL_SHOWCASE_MTK_H */
