#include "common/app_shell.h"
#include <bean/webkit.h>
#include <stdio.h>

static NSView g_loading_label = NULL;

static void bean_webkit_layout(NSView content, CGSize size) {
    (void)content;

    if (g_loading_label) {
        CGFloat label_width = 240.0;
        CGFloat label_height = 32.0;
        NSView_SetFrame(
            g_loading_label,
            CGRectMake(
                (size.width - label_width) * 0.5,
                (size.height - label_height) * 0.5,
                label_width,
                label_height));
    }
}

static void bean_webkit_hide_loading(id self, SEL _cmd, WKWebView web_view, id navigation) {
    (void)self;
    (void)_cmd;
    (void)web_view;
    (void)navigation;
    if (g_loading_label) {
        NSView_SetHidden(g_loading_label, YES);
    }
}

static void bean_webkit_hide_loading_on_error(
    id self,
    SEL _cmd,
    WKWebView web_view,
    id navigation,
    NSError error
) {
    (void)self;
    (void)_cmd;
    (void)web_view;
    (void)navigation;
    (void)error;
    if (g_loading_label) {
        NSView_SetHidden(g_loading_label, YES);
    }
}

static id bean_webkit_navigation_delegate(void) {
    static id delegate = NULL;
    if (delegate) {
        return delegate;
    }

    static const bean_method_spec methods[] = {
        {
            "webView:didFinishNavigation:",
            (IMP)bean_webkit_hide_loading,
            "v@:@@",
        },
        {
            "webView:didFailProvisionalNavigation:withError:",
            (IMP)bean_webkit_hide_loading_on_error,
            "v@:@@@",
        },
        {
            "webView:didFailNavigation:withError:",
            (IMP)bean_webkit_hide_loading_on_error,
            "v@:@@@",
        },
    };

    bean_class cls = bean_make_subclass(
        "BeanWebKitNavigationDelegate",
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

static NSView bean_webkit_loading_label(void) {
    NSTextField label = (NSTextField)NSTextField_LabelWithString(
        bean_str("Loading..."));
    NSTextField_SetBezeled(label, NO);
    NSTextField_SetDrawsBackground(label, NO);
    NSTextField_SetEditable(label, NO);
    NSTextField_SetSelectable(label, NO);
    NSControl_SetAlignment((NSControl)label, NSTextAlignmentCenter);
    return (NSView)label;
}

int main(void) {
    BEAN_AUTORELEASE_POOL {
        NSWindow window = bean_example_window("Bean WebKit", 800.0, 600.0);
        NSView content = NSWindow_ContentView(window);
        CGRect bounds = NSView_Bounds(content);

        WKWebViewConfiguration config = (WKWebViewConfiguration)bean_alloc(bean_cls("WKWebViewConfiguration"));
        config = (WKWebViewConfiguration)bean_init(config);

        WKWebView web_view = (WKWebView)bean_alloc(bean_cls("WKWebView"));
        web_view = (WKWebView)WKWebView_InitWithFrame_Configuration(web_view, bounds, config);
        bean_example_add_fill_subview(content, (NSView)web_view);

        g_loading_label = bean_webkit_loading_label();
        NSView_AddSubview(content, g_loading_label);
        bean_example_set_layout_handler(bean_webkit_layout);
        bean_example_layout_subviews(content);

        WKWebView_SetNavigationDelegate(web_view, bean_webkit_navigation_delegate());

        NSURL url = NSURL_URLWithString(bean_str("https://github.com/hubastard/bean"));
        NSURLRequest request = (NSURLRequest)NSURLRequest_RequestWithURL(url);
        WKWebView_LoadRequest(web_view, request);

        printf("WebKit: loading https://github.com/hubastard/bean\n");
        bean_example_run(window);

        g_loading_label = NULL;
    }
    return 0;
}
