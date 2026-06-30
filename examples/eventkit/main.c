#include "common/cli_async.h"
#include <bean/eventkit.h>
#include <bean/runtime.h>
#include <stdio.h>

static const char* bean_eventkit_auth_status_name(EKAuthorizationStatus status) {
    switch ((int)status) {
    case EKAuthorizationStatusNotDetermined:
        return "not determined (permission not requested yet)";
    case EKAuthorizationStatusRestricted:
        return "restricted (parental controls or policy)";
    case EKAuthorizationStatusDenied:
        return "denied";
    case EKAuthorizationStatusAuthorized: /* same value as FullAccess (3) */
        return "full access";
    case EKAuthorizationStatusWriteOnly:
        return "write only";
    default:
        return "unknown";
    }
}

static int bean_eventkit_can_read_events(EKAuthorizationStatus status) {
    return status == EKAuthorizationStatusAuthorized
        || status == EKAuthorizationStatusFullAccess;
}

typedef struct {
    volatile int done;
    BOOL granted;
    NSError error;
} bean_eventkit_access_state;

static void bean_eventkit_access_done(void* context, void* block, long granted, bean_id error_obj) {
    bean_eventkit_access_state* state = (bean_eventkit_access_state*)context;

    (void)block;

    state->granted = (BOOL)granted;
    state->error = (NSError)error_obj;
    state->done = 1;
}

static BOOL bean_eventkit_request_access(EKEventStore store) {
    bean_eventkit_access_state state = { 0, NO, NULL };
    void* completion;

    bean_cli_prepare_permission_prompt();
    completion = bean_block_create_2(&state, bean_eventkit_access_done);
    EKEventStore_RequestFullAccessToEventsWithCompletion(store, completion);
    bean_cli_wait(&state.done);

    if (!state.granted) {
        const char* message = "access denied";
        if (state.error) {
            message = bean_nsstring_to_utf8(NSError_LocalizedDescription(state.error));
        }
        fprintf(stderr, "Calendar access request failed: %s\n", message ? message : "access denied");
        return NO;
    }

    return YES;
}

static NSString bean_eventkit_format_date(NSDate date) {
    NSDateFormatter formatter = NSDateFormatter_New();
    NSDateFormatter_SetDateStyle(formatter, NSDateFormatterShortStyle);
    NSDateFormatter_SetTimeStyle(formatter, NSDateFormatterShortStyle);
    return NSDateFormatter_StringFromDate(formatter, date);
}

static int bean_eventkit_print_upcoming(EKEventStore store, int limit) {
    NSDate now;
    NSDate end;
    NSPredicate predicate;
    NSArray events;
    NSUInteger count;
    NSUInteger i;
    NSDateFormatter day_formatter;
    NSString day_key;
    const char* day_utf8;

    now = NSDate_Date();
    end = (NSDate)NSDate_DateByAddingTimeInterval(now, 7.0 * 24.0 * 60.0 * 60.0);
    predicate = EKEventStore_PredicateForEventsWithStartDate_EndDate_Calendars(
        store,
        now,
        end,
        NULL);
    events = EKEventStore_EventsMatchingPredicate(store, predicate);
    count = events ? NSArray_Count(events) : 0;

    day_formatter = NSDateFormatter_New();
    NSDateFormatter_SetDateStyle(day_formatter, NSDateFormatterFullStyle);
    NSDateFormatter_SetTimeStyle(day_formatter, NSDateFormatterNoStyle);

    printf("Upcoming events in the next 7 days:\n");
    if (count == 0) {
        printf("  (no events found)\n");
        return 0;
    }

    day_key = NULL;
    day_utf8 = NULL;

    for (i = 0; i < count && (int)i < limit; ++i) {
        EKEvent event = (EKEvent)NSArray_ObjectAtIndex(events, i);
        NSString title = EKCalendarItem_Title((EKCalendarItem)event);
        NSString start_text;
        NSString next_day_key;
        const char* title_utf8;
        const char* start_utf8;
        EKCalendar calendar;
        NSString calendar_title;
        const char* calendar_utf8;

        next_day_key = NSDateFormatter_StringFromDate(day_formatter, EKEvent_StartDate(event));
        if (!day_key || !NSString_IsEqualToString(day_key, next_day_key)) {
            day_key = next_day_key;
            day_utf8 = bean_nsstring_to_utf8(day_key);
            printf("\n%s\n", day_utf8 ? day_utf8 : "(unknown date)");
        }

        start_text = bean_eventkit_format_date(EKEvent_StartDate(event));
        title_utf8 = bean_nsstring_to_utf8(title);
        start_utf8 = bean_nsstring_to_utf8(start_text);
        calendar = EKCalendarItem_Calendar((EKCalendarItem)event);
        calendar_title = calendar ? EKCalendar_Title(calendar) : NULL;
        calendar_utf8 = bean_nsstring_to_utf8(calendar_title);

        printf(
            "  • %s — %s",
            start_utf8 ? start_utf8 : "??:??",
            title_utf8 && title_utf8[0] != '\0' ? title_utf8 : "(untitled)");
        if (calendar_utf8 && calendar_utf8[0] != '\0') {
            printf(" [%s]", calendar_utf8);
        }
        if (EKEvent_IsAllDay(event)) {
            printf(" (all day)");
        }
        printf("\n");
    }

    if (count > (NSUInteger)limit) {
        printf("\nShowing %d of %lu event(s).\n", limit, (unsigned long)count);
    } else {
        printf("\nShowing %lu event(s).\n", (unsigned long)count);
    }

    return 0;
}

static int bean_eventkit_print_calendars(EKEventStore store) {
    NSArray calendars;
    NSUInteger count;
    NSUInteger i;

    calendars = EKEventStore_CalendarsForEntityType(store, EKEntityTypeEvent);
    count = calendars ? NSArray_Count(calendars) : 0;

    printf("Readable calendars (%lu):\n", (unsigned long)count);
    if (count == 0) {
        printf("  (none)\n");
        return 0;
    }

    for (i = 0; i < count; ++i) {
        EKCalendar calendar = (EKCalendar)NSArray_ObjectAtIndex(calendars, i);
        NSString title = EKCalendar_Title(calendar);
        const char* title_utf8 = bean_nsstring_to_utf8(title);
        printf("  • %s\n", title_utf8 && title_utf8[0] != '\0' ? title_utf8 : "(untitled)");
    }

    return 0;
}

int main(void) {
    autoreleasepool {
        EKAuthorizationStatus status;
        EKEventStore store;

        status = EKEventStore_AuthorizationStatusForEntityType(EKEntityTypeEvent);
        printf(
            "Calendar authorization: %s\n",
            bean_eventkit_auth_status_name(status));

        store = EKEventStore_New();

        if (!bean_eventkit_can_read_events(status)) {
            if (status == EKAuthorizationStatusDenied || status == EKAuthorizationStatusRestricted) {
                fprintf(
                    stderr,
                    "Grant Calendar access in System Settings > Privacy & Security > Calendars.\n");
                return 1;
            }

            if (status == EKAuthorizationStatusWriteOnly) {
                fprintf(stderr, "Calendar access is write-only; full read access is required.\n");
                return 1;
            }

            printf("Requesting Calendar access (check for a system prompt)...\n");
            if (!bean_eventkit_request_access(store)) {
                return 1;
            }

            status = EKEventStore_AuthorizationStatusForEntityType(EKEntityTypeEvent);
            printf("Calendar authorization: %s\n", bean_eventkit_auth_status_name(status));
        }

        if (!bean_eventkit_can_read_events(status)) {
            fprintf(stderr, "Calendar read access is still unavailable.\n");
            return 1;
        }

        if (bean_eventkit_print_calendars(store) != 0) {
            return 1;
        }

        printf("\n");
        if (bean_eventkit_print_upcoming(store, 15) != 0) {
            return 1;
        }
    }
    return 0;
}
