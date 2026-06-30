#include <stdio.h>

#include <bean/foundation.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSString hello = NSString_StringWithUTF8String("Hello from bean");
    const char* text = bean_nsstring_to_utf8(hello);
    printf("NSString: %s\n", text ? text : "(null)");

    NSArray array = NSArray_ArrayWithObject(hello);
    NSUInteger count = NSArray_Count(array);
    printf("NSArray count: %lu\n", (unsigned long)count);

    bean_autorelease_pool_pop(pool);
    return 0;
}
