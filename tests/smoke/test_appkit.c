#include <stdio.h>

#include <bean/appkit.h>

int main(void) {
    autoreleasepool {
        NSApplication app = NSApplication_SharedApplication();
        NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyRegular);

        NSWindow window = NSWindow_InitWithContentRect_StyleMask_Backing_Defer(
            (NSWindow)bean_alloc(bean_cls("NSWindow")),
            CGRectMake(0, 0, 320, 240),
            NSWindowStyleMaskTitled | NSWindowStyleMaskClosable,
            NSBackingStoreBuffered,
            NO);

        NSWindow_SetTitle(window, NSString_StringWithUTF8String("Bean AppKit smoke test"));
        NSWindow_MakeKeyAndOrderFront(window, NULL);

        printf("AppKit smoke test: window created\n");
    }
    return 0;
}
