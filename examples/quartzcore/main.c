#include "common/app_shell.h"
#include <bean/quartzcore.h>
#include <stdio.h>

static void bean_quartzcore_layout(NSView content, CGSize size) {
    CALayer layer;
    (void)size;

    layer = NSView_Layer(content);
    if (layer) {
        CALayer_SetFrame(layer, NSView_Bounds(content));
    }
}

int main(void) {
    BEAN_AUTORELEASE_POOL {
        NSWindow window = bean_example_window("Bean QuartzCore", 480.0, 320.0);
        NSView content = NSWindow_ContentView(window);
        NSView_SetWantsLayer(content, YES);

        CALayer layer = NSView_Layer(content);
        NSColor color = NSColor_ColorWithSRGBRed_Green_Blue_Alpha(0.15, 0.35, 0.85, 1.0);
        CALayer_SetBackgroundColor(layer, NSColor_CGColor(color));
        bean_example_set_layout_handler(bean_quartzcore_layout);
        bean_quartzcore_layout(content, NSView_Bounds(content).size);

        printf("QuartzCore: set layer background on window content view.\n");
        bean_example_run(window);
    }
    return 0;
}
