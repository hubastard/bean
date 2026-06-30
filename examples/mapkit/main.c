#include "common/app_shell.h"
#include <bean/mapkit.h>
#include <stdio.h>

int main(void) {
    autoreleasepool {
        NSWindow window = bean_example_window("Bean MapKit", 640.0, 480.0);
        NSView content = NSWindow_ContentView(window);
        CGRect bounds = NSView_Bounds(content);

        MKMapView map_view = (MKMapView)bean_alloc(bean_cls("MKMapView"));
        map_view = (MKMapView)NSView_InitWithFrame((NSView)map_view, bounds);
        bean_example_add_fill_subview(content, (NSView)map_view);

        MKCoordinateRegion region;
        region.center.latitude = 37.7749;
        region.center.longitude = -122.4194;
        region.span.latitudeDelta = 0.2;
        region.span.longitudeDelta = 0.2;
        MKMapView_SetRegion_Animated(map_view, region, NO);

        printf("MapKit: showing map centered on San Francisco.\n");
        bean_example_run(window);
    }
    return 0;
}
