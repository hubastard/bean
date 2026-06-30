#include <bean/foundation.h>
#include <stdio.h>

int main(void) {
    autoreleasepool {
        NSString greeting = bean_str("Hello from Bean Foundation");
        printf("%s\n", bean_nsstring_to_utf8(greeting));
    }
    return 0;
}
