#include <stdio.h>

#include <bean/gameplaykit.h>

int main(void) {
    BEAN_AUTORELEASE_POOL {
        GKRandomDistribution dice = (GKRandomDistribution)GKRandomDistribution_DistributionForDieWithSideCount(20);
        NSInteger roll = GKRandomDistribution_NextInt(dice);
        printf("GameplayKit d20 roll: %ld\n", (long)roll);
    }
    return 0;
}
