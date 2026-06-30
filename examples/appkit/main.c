#include "common/app_shell.h"
#include <bean/appkit.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>

static NSView g_preview_view = NULL;
static NSTextField g_status_label = NULL;
static NSTextField g_title_label = NULL;
static NSSlider g_hue_slider = NULL;
static NSButton g_random_button = NULL;
static NSButton g_shape_button = NULL;
static int g_use_circle = 1;

static void bean_appkit_refresh_status(CGFloat hue) {
    NSString text = NSString_StringWithFormat(
        bean_str("Hue: %.0f°  •  drag slider or press Random Color"),
        hue * 360.0);
    NSControl_SetStringValue((NSControl)g_status_label, text);
}

static void bean_appkit_redraw_preview(void) {
    if (g_preview_view) {
        NSView_SetNeedsDisplay(g_preview_view, YES);
    }
}

static void bean_appkit_preview_draw(id self, SEL _cmd, CGRect dirty_rect) {
    CGRect bounds;
    CGRect inset;
    NSColor background;
    NSColor fill;
    NSColor stroke;
    NSBezierPath path;
    CGFloat hue;

    (void)dirty_rect;

    bounds = NSView_Bounds((NSView)self);
    background = NSColor_ColorWithCalibratedWhite_Alpha(0.14, 1.0);
    NSColor_SetFill(background);
    NSBezierPath_FillRect(bounds);

    hue = g_hue_slider ? (CGFloat)NSControl_DoubleValue((NSControl)g_hue_slider) : 0.58;
    fill = NSColor_ColorWithCalibratedHue_Saturation_Brightness_Alpha(hue, 0.78, 0.96, 1.0);
    NSColor_SetFill(fill);

    inset = bounds;
    inset.origin.x += 28.0;
    inset.origin.y += 28.0;
    inset.size.width -= 56.0;
    inset.size.height -= 56.0;

    if (g_use_circle) {
        path = NSBezierPath_BezierPathWithOvalInRect(inset);
    } else {
        path = NSBezierPath_BezierPathWithRoundedRect_XRadius_YRadius(inset, 28.0, 28.0);
    }
    NSBezierPath_Fill(path);

    stroke = NSColor_ColorWithCalibratedWhite_Alpha(1.0, 0.55);
    NSColor_SetStroke(stroke);
    NSBezierPath_SetLineWidth(path, 3.0);
    NSBezierPath_Stroke(path);

    {
        bean_super_call_rect(self, bean_cls("NSView"), _cmd, dirty_rect);
    }
}

static void bean_appkit_hue_changed(id self, SEL _cmd, NSSlider slider) {
    CGFloat hue;

    (void)self;
    (void)_cmd;

    hue = (CGFloat)NSControl_DoubleValue((NSControl)slider);
    bean_appkit_refresh_status(hue);
    bean_appkit_redraw_preview();
}

static void bean_appkit_random_color(id self, SEL _cmd, id sender) {
    CGFloat hue;

    (void)self;
    (void)_cmd;
    (void)sender;

    hue = (CGFloat)arc4random_uniform(10000) / 10000.0;
    NSControl_SetDoubleValue((NSControl)g_hue_slider, hue);
    bean_appkit_hue_changed(self, _cmd, g_hue_slider);
}

static void bean_appkit_toggle_shape(id self, SEL _cmd, id sender) {
    (void)self;
    (void)_cmd;
    (void)sender;

    g_use_circle = !g_use_circle;
    bean_appkit_redraw_preview();
}

static id bean_appkit_controller(void) {
    static id controller = NULL;

    if (controller) {
        return controller;
    }

    static const bean_method_spec methods[] = {
        { "hueChanged:", (IMP)bean_appkit_hue_changed, "v@:@" },
        { "randomColor:", (IMP)bean_appkit_random_color, "v@:@" },
        { "toggleShape:", (IMP)bean_appkit_toggle_shape, "v@:@" },
    };

    bean_class cls = bean_make_subclass(
        "BeanAppKitController",
        bean_cls("NSObject"),
        methods,
        sizeof(methods) / sizeof(methods[0]),
        0);
    if (!cls) {
        return NULL;
    }

    controller = bean_init((id)bean_alloc(cls));
    return controller;
}

static void bean_appkit_layout(NSView content, CGSize size) {
    (void)content;

    if (g_title_label) {
        NSView_SetFrame(
            (NSView)g_title_label,
            CGRectMake(24.0, size.height - 52.0, size.width - 48.0, 28.0));
    }
    if (g_preview_view) {
        NSView_SetFrame(
            g_preview_view,
            CGRectMake(32.0, 132.0, size.width - 64.0, size.height - 220.0));
    }
    if (g_hue_slider) {
        NSView_SetFrame(
            (NSView)g_hue_slider,
            CGRectMake(32.0, 92.0, size.width - 64.0, 24.0));
    }
    if (g_status_label) {
        NSView_SetFrame(
            (NSView)g_status_label,
            CGRectMake(24.0, 58.0, size.width - 48.0, 22.0));
    }
    if (g_random_button) {
        NSView_SetFrame((NSView)g_random_button, CGRectMake(32.0, 16.0, 150.0, 32.0));
    }
    if (g_shape_button) {
        NSView_SetFrame(
            (NSView)g_shape_button,
            CGRectMake(size.width - 182.0, 16.0, 150.0, 32.0));
    }
    bean_appkit_redraw_preview();
}

static NSView bean_appkit_build_ui(NSView content) {
    id controller;
    NSTextField title;
    NSSlider slider;
    NSView preview;
    static bean_class preview_cls = NULL;

    controller = bean_appkit_controller();

    title = (NSTextField)NSTextField_LabelWithString(
        bean_str("Bean AppKit Showcase"));
    NSTextField_SetBezeled(title, NO);
    NSTextField_SetDrawsBackground(title, NO);
    NSTextField_SetEditable(title, NO);
    NSTextField_SetSelectable(title, NO);
    NSControl_SetAlignment((NSControl)title, NSTextAlignmentCenter);
    NSView_AddSubview(content, (NSView)title);
    g_title_label = title;

    if (!preview_cls) {
        static const bean_method_spec preview_methods[] = {
            { "drawRect:", (IMP)bean_appkit_preview_draw, "v@:{CGRect={CGPoint=dd}{CGSize=dd}}" },
        };
        preview_cls = bean_make_subclass(
            "BeanAppKitPreviewView",
            bean_cls("NSView"),
            preview_methods,
            sizeof(preview_methods) / sizeof(preview_methods[0]),
            0);
    }

    preview = (NSView)bean_init((id)bean_alloc(preview_cls));
    NSView_AddSubview(content, preview);
    g_preview_view = preview;

    slider = (NSSlider)NSSlider_SliderWithValue_MinValue_MaxValue_Target_Action(
        0.58,
        0.0,
        1.0,
        controller,
        bean_sel("hueChanged:"));
    NSControl_SetContinuous((NSControl)slider, YES);
    NSView_AddSubview(content, (NSView)slider);
    g_hue_slider = slider;

    g_status_label = (NSTextField)NSTextField_LabelWithString(
        bean_str("Hue: 209°  •  drag slider or press Random Color"));
    NSTextField_SetBezeled(g_status_label, NO);
    NSTextField_SetDrawsBackground(g_status_label, NO);
    NSTextField_SetEditable(g_status_label, NO);
    NSTextField_SetSelectable(g_status_label, NO);
    NSControl_SetAlignment((NSControl)g_status_label, NSTextAlignmentCenter);
    NSView_AddSubview(content, (NSView)g_status_label);

    g_random_button = (NSButton)NSButton_ButtonWithTitle_Target_Action(
        bean_str("Random Color"),
        controller,
        bean_sel("randomColor:"));
    NSView_AddSubview(content, (NSView)g_random_button);

    g_shape_button = (NSButton)NSButton_ButtonWithTitle_Target_Action(
        bean_str("Toggle Shape"),
        controller,
        bean_sel("toggleShape:"));
    NSView_AddSubview(content, (NSView)g_shape_button);

    bean_appkit_layout(content, NSView_Bounds(content).size);
    return preview;
}

int main(void) {
    autoreleasepool {
        NSWindow window = bean_example_window("Bean AppKit", 640.0, 480.0);
        NSView content = NSWindow_ContentView(window);

        bean_appkit_build_ui(content);
        bean_example_set_layout_handler(bean_appkit_layout);
        bean_appkit_refresh_status(0.58);

        printf("AppKit: color preview UI ready. Drag the hue slider or use the buttons.\n");
        bean_example_run(window);
    }
    return 0;
}
