#include <stdio.h>

#include <bean/storekit.h>

int main(void) {
    BEAN_AUTORELEASE_POOL {
        BOOL can_pay = SKPaymentQueue_CanMakePayments();
        printf("StoreKit can make payments: %s\n", can_pay ? "yes" : "no");
    }
    return 0;
}
