#ifndef BEAN_EXAMPLES_APP_SHELL_H
#define BEAN_EXAMPLES_APP_SHELL_H

#include <bean/appkit.h>
#include <stdio.h>

typedef void (*bean_example_layout_fn)(NSView content, CGSize size);

static bean_example_layout_fn g_bean_example_layout = NULL;
static bean_class g_bean_example_content_cls = NULL;

static void bean_example_app_send_event(id self, SEL _cmd, NSEvent event) {
    if (NSEvent_Type(event) == NSEventTypeKeyDown && NSEvent_KeyCode(event) == 53u) {
        NSApplication_Terminate((NSApplication)self, NULL);
        return;
    }

    {
        struct objc_super super_info = { self, (Class)bean_cls("NSApplication") };
        ((void (*)(struct objc_super*, SEL, NSEvent))objc_msgSendSuper)(&super_info, _cmd, event);
    }
}

static void bean_example_content_resize(id self, SEL _cmd, CGSize old_size) {
    {
        struct objc_super super_info = { self, (Class)bean_cls("NSView") };
        ((void (*)(struct objc_super*, SEL, CGSize))objc_msgSendSuper)(&super_info, _cmd, old_size);
    }

    if (g_bean_example_layout) {
        CGRect bounds = NSView_Bounds((NSView)self);
        g_bean_example_layout((NSView)self, bounds.size);
    }
}

static inline bean_class bean_example_content_view_class(void) {
    if (!g_bean_example_content_cls) {
        static const bean_method_spec methods[] = {
            { "resizeSubviewsWithOldSize:", (IMP)bean_example_content_resize, "v@:{CGSize=dd}" },
        };

        g_bean_example_content_cls = bean_make_subclass(
            "BeanExampleContentView",
            bean_cls("NSView"),
            methods,
            sizeof(methods) / sizeof(methods[0]),
            0);
    }

    return g_bean_example_content_cls;
}

static inline NSView bean_example_install_content_view(NSWindow window) {
    NSView previous = NSWindow_ContentView(window);
    CGRect bounds = NSView_Bounds(previous);
    NSView content;

    content = (NSView)bean_init((id)bean_alloc(bean_example_content_view_class()));
    content = (NSView)NSView_InitWithFrame(content, bounds);
    NSView_SetAutoresizesSubviews(content, YES);
    NSWindow_SetContentView(window, content);
    return content;
}

static inline void bean_example_set_layout_handler(bean_example_layout_fn handler) {
    g_bean_example_layout = handler;
}

static inline void bean_example_layout_subviews(NSView content) {
    if (g_bean_example_layout) {
        CGRect bounds = NSView_Bounds(content);
        g_bean_example_layout(content, bounds.size);
    }
}

static inline void bean_example_add_fill_subview(NSView content, NSView view) {
    CGRect bounds = NSView_Bounds(content);

    NSView_SetFrame(view, bounds);
    NSView_SetAutoresizingMask(view, NSViewWidthSizable | NSViewHeightSizable);
    NSView_AddSubview(content, view);
}

static inline void bean_example_install_esc_to_quit(void) {
    static int installed = 0;
    static bean_class esc_app_cls = NULL;

    if (installed) {
        return;
    }

    if (!esc_app_cls) {
        static const bean_method_spec methods[] = {
            { "sendEvent:", (IMP)bean_example_app_send_event, "v@:@" },
        };

        esc_app_cls = bean_make_subclass(
            "BeanExampleApplication",
            bean_cls("NSApplication"),
            methods,
            sizeof(methods) / sizeof(methods[0]),
            0);
    }

    if (esc_app_cls) {
        object_setClass(NSApplication_SharedApplication(), esc_app_cls);
    }

    installed = 1;
}

static inline NSWindow bean_example_window(const char* title, CGFloat width, CGFloat height) {
    NSApplication app = NSApplication_SharedApplication();
    NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyRegular);

    CGRect frame = CGRectMake(100.0, 100.0, width, height);
    NSWindow window = NSWindow_InitWithContentRect_StyleMask_Backing_Defer(
        (NSWindow)bean_alloc(bean_cls("NSWindow")),
        frame,
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable,
        NSBackingStoreBuffered,
        NO);

    NSWindow_SetTitle(window, NSString_StringWithUTF8String(title));
    NSWindow_Center(window);
    bean_example_install_content_view(window);
    return window;
}

static inline void bean_example_run(NSWindow window) {
    NSWindow_MakeKeyAndOrderFront(window, NULL);
    NSApplication_ActivateIgnoringOtherApps(NSApplication_SharedApplication(), YES);
    bean_example_install_esc_to_quit();
    NSApplication_Run(NSApplication_SharedApplication());
}

#endif /* BEAN_EXAMPLES_APP_SHELL_H */
