#include <stdio.h>

#include <bean/cloudkit.h>

int main(void) {
    autoreleasepool {
        /*
        * CKContainer APIs require a signed app with CloudKit entitlements.
        * This CLI example builds records locally instead.
        */
        printf("CloudKit: building a sample record (no iCloud account required).\n");

        CKRecordID record_id = CKRecordID_Alloc();
        record_id = (CKRecordID)CKRecordID_InitWithRecordName(
            record_id,
            bean_str("demo-record"));
        printf("Record name: %s\n", bean_nsstring_to_utf8(CKRecordID_RecordName(record_id)));

        CKRecord record = CKRecord_Alloc();
        record = (CKRecord)CKRecord_InitWithRecordType_RecordID(
            record,
            bean_str("Task"),
            record_id);

        CKRecord_SetObject_ForKey(record, bean_str("Learn CloudKit with Bean"), bean_str("title"));

        printf("Record type: %s\n", bean_nsstring_to_utf8(CKRecord_RecordType(record)));
        printf("Record title: %s\n", bean_nsstring_to_utf8((NSString)CKRecord_ObjectForKey(record, bean_str("title"))));
    }
    return 0;
}
