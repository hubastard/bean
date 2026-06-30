#include "common/cli_async.h"
#include <bean/contacts.h>
#include <bean/runtime.h>
#include <stdio.h>

static const char* bean_contacts_auth_status_name(CNAuthorizationStatus status) {
    switch ((int)status) {
    case CNAuthorizationStatusNotDetermined:
        return "not determined (permission not requested yet)";
    case CNAuthorizationStatusRestricted:
        return "restricted (parental controls or policy)";
    case CNAuthorizationStatusDenied:
        return "denied";
    case CNAuthorizationStatusAuthorized:
        return "authorized";
    case CNAuthorizationStatusLimited:
        return "limited";
    default:
        return "unknown";
    }
}

static int bean_contacts_is_authorized(CNAuthorizationStatus status) {
    return status == CNAuthorizationStatusAuthorized || status == CNAuthorizationStatusLimited;
}

typedef struct {
    volatile int done;
    BOOL granted;
    NSError error;
} bean_contacts_access_state;

static void bean_contacts_access_done(void* context, void* block, long granted, bean_id error_obj) {
    bean_contacts_access_state* state = (bean_contacts_access_state*)context;

    (void)block;

    state->granted = (BOOL)granted;
    state->error = (NSError)error_obj;
    state->done = 1;
}

static BOOL bean_contacts_request_access(CNContactStore store) {
    bean_contacts_access_state state = { 0, NO, NULL };
    void* completion;

    bean_cli_prepare_permission_prompt();
    completion = bean_block_create_2(&state, bean_contacts_access_done);
    CNContactStore_RequestAccessForEntityType_CompletionHandler(
        store,
        CNEntityTypeContacts,
        completion);

    bean_cli_wait(&state.done);

    if (!state.granted) {
        const char* message = "access denied";
        if (state.error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(state.error));
        }
        fprintf(stderr, "Contacts access request failed: %s\n", message ? message : "access denied");
        return NO;
    }

    return YES;
}

typedef struct {
    int printed;
    int limit;
} bean_contacts_list_state;

static void bean_contacts_print_contact(
    void* context,
    void* block,
    bean_id contact_obj,
    bean_id stop_obj
) {
    bean_contacts_list_state* state = (bean_contacts_list_state*)context;
    CNContact contact = (CNContact)contact_obj;
    BOOL* stop = (BOOL*)(void*)stop_obj;
    NSString name;
    const char* name_utf8;

    (void)block;

    if (state->printed >= state->limit) {
        if (stop) {
            *stop = YES;
        }
        return;
    }

    name = CNContactFormatter_StringFromContact_Style(contact, CNContactFormatterStyleFullName);
    name_utf8 = bean_nsstring_to_utf8(name);
    if (!name_utf8 || name_utf8[0] == '\0') {
        name_utf8 = "(no name)";
    }

    printf("  %2d. %s\n", state->printed + 1, name_utf8);
    state->printed += 1;
}

static int bean_contacts_print_sample(CNContactStore store, int limit) {
    CNContactFetchRequest request;
    id key_descriptor;
    id keys[1];
    NSArray keys_to_fetch;
    bean_contacts_list_state state = { 0, limit };
    void* block;
    NSError error;
    BOOL ok;

    key_descriptor = CNContactFormatter_DescriptorForRequiredKeysForStyle(
        CNContactFormatterStyleFullName);
    keys[0] = key_descriptor;
    keys_to_fetch = NSArray_ArrayWithObjects_Count(keys, 1);

    request = CNContactFetchRequest_InitWithKeysToFetch(
        (CNContactFetchRequest)bean_alloc(bean_cls("CNContactFetchRequest")),
        keys_to_fetch);

    block = bean_block_create_2_id(&state, bean_contacts_print_contact);
    error = NULL;
    ok = CNContactStore_EnumerateContactsWithFetchRequest_Error_UsingBlock(
        store,
        request,
        &error,
        block);

    if (!ok) {
        const char* message = "enumeration failed";
        if (error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(error));
        }
        fprintf(stderr, "Contacts enumeration failed: %s\n", message ? message : "enumeration failed");
        return 1;
    }

    if (state.printed == 0) {
        printf("  (no contacts found)\n");
    } else {
        printf("Showing %d contact(s).\n", state.printed);
    }

    return 0;
}

int main(void) {
    BEAN_AUTORELEASE_POOL {
        CNAuthorizationStatus status;
        CNContactStore store;

        status = CNContactStore_AuthorizationStatusForEntityType(CNEntityTypeContacts);
        printf(
            "Contacts authorization: %s\n",
            bean_contacts_auth_status_name(status));

        store = (CNContactStore)bean_new(bean_cls("CNContactStore"));

        if (!bean_contacts_is_authorized(status)) {
            if (status == CNAuthorizationStatusDenied || status == CNAuthorizationStatusRestricted) {
                fprintf(
                    stderr,
                    "Grant Contacts access in System Settings > Privacy & Security > Contacts.\n");
                return 1;
            }

            printf("Requesting Contacts access (check for a system prompt)...\n");
            if (!bean_contacts_request_access(store)) {
                return 1;
            }
        }

        printf("Sample contacts:\n");
        if (bean_contacts_print_sample(store, 10) != 0) {
            return 1;
        }
    }
    return 0;
}
