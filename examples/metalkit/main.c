#include "common/app_shell.h"
#include "common/metal_showcase_mtk.h"
#include <bean/metalkit.h>
#include <stdio.h>

int main(void) {
    BEAN_AUTORELEASE_POOL {
        NSWindow window = bean_example_window("Bean MetalKit", 640.0, 480.0);
        NSView content = NSWindow_ContentView(window);
        CGRect bounds = NSView_Bounds(content);

        id device = MTLCreateSystemDefaultDevice();
        MTKView metal_view = (MTKView)bean_alloc(bean_cls("MTKView"));
        metal_view = (MTKView)MTKView_InitWithFrame_Device(metal_view, bounds, device);
        bean_example_add_fill_subview(content, (NSView)metal_view);
        bean_metal_showcase_configure_mtk_view(metal_view);

        printf("MetalKit: animated gradient triangle via MTKView delegate.\n");
        bean_example_run(window);
    }
    return 0;
}
