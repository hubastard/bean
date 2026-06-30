#include <stdio.h>

#include <bean/gameplaykit.h>

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    GKRandomDistribution dice = (GKRandomDistribution)GKRandomDistribution_DistributionForDieWithSideCount(20);
    NSInteger roll = GKRandomDistribution_NextInt(dice);
    printf("GameplayKit d20 roll: %ld\n", (long)roll);

    bean_autorelease_pool_pop(pool);
    return 0;
}
