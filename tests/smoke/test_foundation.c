#include <stdio.h>

#include <bean/foundation.h>

int main(void) {
    autoreleasepool {
        NSString hello = NSString_StringWithUTF8String("Hello from bean");
        const char* text = bean_nsstring_to_utf8(hello);
        printf("NSString: %s\n", text ? text : "(null)");

        NSArray array = NSArray_ArrayWithObject(hello);
        NSUInteger count = NSArray_Count(array);
        printf("NSArray count: %lu\n", (unsigned long)count);
    }
    return 0;
}
