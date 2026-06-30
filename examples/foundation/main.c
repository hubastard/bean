#include <stdio.h>

#include <bean/foundation.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSString greeting = NSString_StringWithUTF8String("Hello from Bean Foundation");
    const char* text = bean_nsstring_to_utf8(greeting);
    printf("NSString: %s\n", text ? text : "(null)");

    NSArray array = NSArray_ArrayWithObject(greeting);
    printf("NSArray count: %lu\n", (unsigned long)NSArray_Count(array));

    bean_autorelease_pool_pop(pool);
    return 0;
}
