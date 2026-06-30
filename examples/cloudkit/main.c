#include <stdio.h>

#include <bean/cloudkit.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    /*
     * CKContainer APIs require a signed app with CloudKit entitlements.
     * This CLI example builds records locally instead.
     */
    printf("CloudKit: building a sample record (no iCloud account required).\n");

    CKRecordID record_id = (CKRecordID)bean_alloc(bean_cls("CKRecordID"));
    record_id = (CKRecordID)CKRecordID_InitWithRecordName(
        record_id,
        NSString_StringWithUTF8String("demo-record"));
    printf(
        "Record name: %s\n",
        bean_nsstring_to_utf8(CKRecordID_RecordName(record_id)));

    CKRecord record = (CKRecord)bean_alloc(bean_cls("CKRecord"));
    record = (CKRecord)CKRecord_InitWithRecordType_RecordID(
        record,
        NSString_StringWithUTF8String("Task"),
        record_id);
    CKRecord_SetObject_ForKey(
        record,
        NSString_StringWithUTF8String("Learn CloudKit with Bean"),
        NSString_StringWithUTF8String("title"));
    printf("Record type: %s\n", bean_nsstring_to_utf8(CKRecord_RecordType(record)));
    printf(
        "Record title: %s\n",
        bean_nsstring_to_utf8(
            (NSString)CKRecord_ObjectForKey(
                record,
                NSString_StringWithUTF8String("title"))));

    bean_autorelease_pool_pop(pool);
    return 0;
}
