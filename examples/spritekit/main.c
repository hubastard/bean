#include "common/app_shell.h"
#include <bean/spritekit.h>
#include <math.h>
#include <stdio.h>

static SKNode bean_spritekit_orbiter(
    SKScene scene,
    CGFloat center_x,
    CGFloat center_y,
    CGFloat radius,
    CGFloat dot_radius,
    NSColor color,
    CGFloat duration,
    CGFloat start_angle
) {
    SKNode orbit = (SKNode)SKNode_Node();
    SKShapeNode dot = (SKShapeNode)SKShapeNode_ShapeNodeWithCircleOfRadius(dot_radius);
    SKAction spin;
    SKAction loop;
    CGPoint orbit_position;
    CGPoint dot_position;

    SKShapeNode_SetFillColor(dot, color);
    SKShapeNode_SetStrokeColor(dot, NSColor_ColorWithCalibratedWhite_Alpha(1.0, 0.35));
    SKShapeNode_SetLineWidth(dot, 2.0);

    orbit_position.x = center_x;
    orbit_position.y = center_y;
    SKNode_SetPosition(orbit, orbit_position);
    SKNode_SetZRotation(orbit, start_angle);

    dot_position.x = radius;
    dot_position.y = 0.0;
    SKNode_SetPosition((SKNode)dot, dot_position);
    SKNode_AddChild(orbit, (SKNode)dot);
    SKNode_AddChild(scene, orbit);

    spin = SKAction_RotateByAngle_Duration((CGFloat)(2.0 * M_PI), duration);
    loop = SKAction_RepeatActionForever(spin);
    SKNode_RunAction(orbit, loop);
    return orbit;
}

static void bean_spritekit_center_logo(SKScene scene, CGFloat center_x, CGFloat center_y) {
    SKShapeNode logo;
    SKAction rotate;
    SKAction grow;
    SKAction shrink;
    SKAction pulse;
    SKAction animate;
    SKAction loop;
    id pulse_steps[2];
    id animate_steps[2];
    CGPoint position;

    logo = (SKShapeNode)SKShapeNode_ShapeNodeWithRectOfSize_CornerRadius(
        (CGSize){ 96.0, 96.0 },
        22.0);
    SKShapeNode_SetFillColor(
        logo,
        NSColor_ColorWithCalibratedRed_Green_Blue_Alpha(0.25, 0.55, 0.95, 1.0));
    SKShapeNode_SetStrokeColor(logo, NSColor_ColorWithCalibratedWhite_Alpha(1.0, 0.8));
    SKShapeNode_SetLineWidth(logo, 3.0);

    position.x = center_x;
    position.y = center_y;
    SKNode_SetPosition((SKNode)logo, position);
    SKNode_AddChild(scene, (SKNode)logo);

    rotate = SKAction_RotateByAngle_Duration((CGFloat)(2.0 * M_PI), 4.0);
    grow = SKAction_ScaleTo_Duration(1.12, 0.7);
    shrink = SKAction_ScaleTo_Duration(1.0, 0.7);
    pulse_steps[0] = grow;
    pulse_steps[1] = shrink;
    pulse = SKAction_Sequence(NSArray_ArrayWithObjects_Count(pulse_steps, 2));
    animate_steps[0] = rotate;
    animate_steps[1] = pulse;
    animate = SKAction_Group(NSArray_ArrayWithObjects_Count(animate_steps, 2));
    loop = SKAction_RepeatActionForever(animate);
    SKNode_RunAction((SKNode)logo, loop);
}

static void bean_spritekit_title(SKScene scene, CGFloat center_x) {
    SKLabelNode title;
    CGPoint position;

    title = (SKLabelNode)SKLabelNode_LabelNodeWithFontNamed(
        NSString_StringWithUTF8String("HelveticaNeue-Bold"));
    SKLabelNode_SetText(title, NSString_StringWithUTF8String("Bean SpriteKit"));
    SKLabelNode_SetFontSize(title, 28.0);
    SKLabelNode_SetFontColor(title, NSColor_ColorWithCalibratedWhite_Alpha(0.92, 1.0));
    SKLabelNode_SetHorizontalAlignmentMode(title, SKLabelHorizontalAlignmentModeCenter);

    position.x = center_x;
    position.y = 36.0;
    SKNode_SetPosition((SKNode)title, position);
    SKNode_AddChild(scene, (SKNode)title);
}

static SKScene bean_spritekit_demo_scene(CGSize size) {
    SKScene scene;
    CGFloat center_x;
    CGFloat center_y;
    static const struct {
        CGFloat r;
        CGFloat g;
        CGFloat b;
        CGFloat orbit;
        CGFloat speed;
        CGFloat angle;
    } orbiters[] = {
        { 0.95, 0.35, 0.45, 150.0, 5.0, 0.0 },
        { 0.35, 0.85, 0.55, 130.0, 4.2, 1.05 },
        { 0.45, 0.65, 0.98, 170.0, 6.5, 2.1 },
        { 0.98, 0.78, 0.25, 110.0, 3.6, 3.3 },
        { 0.72, 0.45, 0.95, 190.0, 7.0, 4.6 },
        { 0.30, 0.90, 0.90, 210.0, 8.0, 5.4 },
    };
    size_t i;

    scene = (SKScene)SKScene_SceneWithSize(size);
    SKScene_SetBackgroundColor(
        scene,
        NSColor_ColorWithCalibratedRed_Green_Blue_Alpha(0.07, 0.09, 0.14, 1.0));
    SKScene_SetScaleMode(scene, SKSceneScaleModeResizeFill);

    center_x = size.width * 0.5;
    center_y = size.height * 0.5;

    bean_spritekit_center_logo(scene, center_x, center_y);
    bean_spritekit_title(scene, center_x);

    for (i = 0; i < sizeof(orbiters) / sizeof(orbiters[0]); ++i) {
        NSColor color = NSColor_ColorWithCalibratedRed_Green_Blue_Alpha(
            orbiters[i].r,
            orbiters[i].g,
            orbiters[i].b,
            1.0);
        bean_spritekit_orbiter(
            scene,
            center_x,
            center_y,
            orbiters[i].orbit,
            12.0,
            color,
            orbiters[i].speed,
            orbiters[i].angle);
    }

    return scene;
}

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSWindow window = bean_example_window("Bean SpriteKit", 640.0, 480.0);
    NSView content = NSWindow_ContentView(window);
    CGRect bounds = NSView_Bounds(content);

    SKView sprite_view = (SKView)bean_alloc(bean_cls("SKView"));
    sprite_view = (SKView)NSView_InitWithFrame((NSView)sprite_view, bounds);
    bean_example_add_fill_subview(content, (NSView)sprite_view);

    SKView_PresentScene(sprite_view, bean_spritekit_demo_scene(bounds.size));
    SKView_SetShowsFPS(sprite_view, YES);
    SKView_SetShowsNodeCount(sprite_view, YES);

    printf("SpriteKit: orbiting colors around a pulsing Bean logo.\n");
    bean_example_run(window);

    bean_autorelease_pool_pop(pool);
    return 0;
}
