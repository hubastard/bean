#include "common/app_shell.h"
#include "common/metal_showcase_layer.h"
#include <bean/metal.h>
#include <stdio.h>

int main(void) {
    BEAN_AUTORELEASE_POOL {
        MTLDevice device;

        device = (MTLDevice)MTLCreateSystemDefaultDevice();
        if (device) {
            NSString name = MTLDevice_Name(device);
            printf(
                "Metal device: %s\n",
                bean_nsstring_to_utf8(name) ? bean_nsstring_to_utf8(name) : "(unknown)");
        }

        NSWindow window = bean_example_window("Bean Metal", 640.0, 480.0);
        NSView content = NSWindow_ContentView(window);
        if (!bean_metal_showcase_attach_metal_layer(content, window)) {
            fprintf(stderr, "Metal: failed to attach CAMetalLayer showcase.\n");
            return 1;
        }

        NSWindow_MakeKeyAndOrderFront(window, NULL);
        NSApplication_ActivateIgnoringOtherApps(NSApplication_SharedApplication(), YES);
        bean_metal_showcase_start_metal_layer_animation();

        printf("Metal: animated gradient triangle via CAMetalLayer + CADisplayLink.\n");
        bean_example_install_esc_to_quit();
        NSApplication_Run(NSApplication_SharedApplication());
    }
    return 0;
}
