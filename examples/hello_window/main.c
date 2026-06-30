#include <bean/appkit.h>
#include <stdio.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSApplication app = NSApplication_SharedApplication();
    NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyRegular);

    CGRect frame = CGRectMake(100.0, 100.0, 640.0, 480.0);
    NSWindow window = NSWindow_InitWithContentRect_StyleMask_Backing_Defer(
        (NSWindow)bean_alloc(bean_cls("NSWindow")),
        frame,
        NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable,
        NSBackingStoreBuffered,
        NO);

    NSWindow_SetTitle(window, NSString_StringWithUTF8String("Bean Hello Window"));
    NSWindow_Center(window);
    NSWindow_MakeKeyAndOrderFront(window, NULL);
    NSApplication_ActivateIgnoringOtherApps(app, YES);

    printf("Bean hello window running. Close the window to exit.\n");
    NSApplication_Run(app);

    bean_autorelease_pool_pop(pool);
    return 0;
}
