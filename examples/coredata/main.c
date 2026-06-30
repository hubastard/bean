#include <stdio.h>

#include <bean/coredata.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSManagedObjectModel model = (NSManagedObjectModel)bean_alloc(bean_cls("NSManagedObjectModel"));
    model = (NSManagedObjectModel)NSManagedObjectModel_Init(model);
    NSURL store_url = NSPersistentContainer_DefaultDirectoryURL();
    const char* path = bean_nsstring_to_utf8((NSString)NSURL_Path(store_url));
    printf("Core Data default store directory: %s\n", path ? path : "(null)");

    bean_autorelease_pool_pop(pool);
    return 0;
}
