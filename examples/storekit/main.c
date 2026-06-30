#include <stdio.h>

#include <bean/storekit.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    BOOL can_pay = SKPaymentQueue_CanMakePayments();
    printf("StoreKit can make payments: %s\n", can_pay ? "yes" : "no");

    bean_autorelease_pool_pop(pool);
    return 0;
}
