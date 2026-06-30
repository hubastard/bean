#ifndef BEAN_EXAMPLES_CLI_ASYNC_H
#define BEAN_EXAMPLES_CLI_ASYNC_H

#include <bean/appkit.h>

static inline void bean_cli_pump_run_loop(void) {
    NSRunLoop_RunUntilDate(
        NSRunLoop_CurrentRunLoop(),
        NSDate_DateWithTimeIntervalSinceNow(0.05));
}

static inline void bean_cli_wait(volatile int* done) {
    while (!*done) {
        bean_cli_pump_run_loop();
    }
}

static inline void bean_cli_prepare_permission_prompt(void) {
    NSApplication app = NSApplication_SharedApplication();
    NSApplication_SetActivationPolicy(app, NSApplicationActivationPolicyAccessory);
}

#endif /* BEAN_EXAMPLES_CLI_ASYNC_H */
