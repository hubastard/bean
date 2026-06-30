#include "common/app_shell.h"
#include <bean/scenekit.h>
#include <math.h>
#include <stdio.h>

static inline void bean_scn_view_set_point_of_view(SCNView view, SCNNode node) {
    ((void (*)(SCNView, SEL, SCNNode))bean_msgSend)(view, bean_sel("setPointOfView:"), node);
}

static inline void bean_scn_node_run_action(SCNNode node, SCNAction action) {
    ((void (*)(SCNNode, SEL, SCNAction))bean_msgSend)(node, bean_sel("runAction:"), action);
}

static SCNNode bean_scn_camera_node(void) {
    SCNNode camera_node = SCNNode_Node();
    SCNCamera camera = (SCNCamera)SCNCamera_Camera();
    SCNNode_SetCamera(camera_node, camera);

    {
        SCNVector3 position = { 0.0f, 0.0f, 5.0f };
        SCNNode_SetPosition(camera_node, position);
    }

    return camera_node;
}

static SCNNode bean_scn_light_node(void) {
    SCNNode light_node = SCNNode_Node();
    SCNLight light = (SCNLight)SCNLight_Light();
    SCNLight_SetType(light, NSString_StringWithUTF8String("omni"));
    SCNLight_SetIntensity(light, 1000.0);
    SCNNode_SetLight(light_node, light);

    {
        SCNVector3 position = { 3.0f, 6.0f, 8.0f };
        SCNNode_SetPosition(light_node, position);
    }

    return light_node;
}

int main(void) {
    bean_autorelease_pool_scope pool = bean_autorelease_pool_push();

    NSWindow window = bean_example_window("Bean SceneKit", 640.0, 480.0);
    NSView content = NSWindow_ContentView(window);
    CGRect bounds = NSView_Bounds(content);

    SCNView scene_view = (SCNView)bean_alloc(bean_cls("SCNView"));
    scene_view = (SCNView)SCNView_InitWithFrame_Options(scene_view, bounds, NULL);
    bean_example_add_fill_subview(content, (NSView)scene_view);

    SCNScene scene = SCNScene_Scene();
    SCNNode root = SCNScene_RootNode(scene);

    SCNGeometry box = (SCNGeometry)SCNBox_BoxWithWidth_Height_Length_ChamferRadius(1.0, 1.0, 1.0, 0.08);
    {
        SCNMaterial material = (SCNMaterial)SCNMaterial_Material();
        NSColor color = NSColor_ColorWithCalibratedRed_Green_Blue_Alpha(0.25, 0.55, 0.95, 1.0);
        SCNMaterialProperty_SetContents(SCNMaterial_Diffuse(material), color);
        SCNGeometry_SetFirstMaterial(box, material);
    }

    SCNNode box_node = SCNNode_Node();
    SCNNode_SetGeometry(box_node, box);
    SCNNode_AddChildNode(root, box_node);

    SCNNode camera_node = bean_scn_camera_node();
    SCNNode_AddChildNode(root, camera_node);
    SCNNode_AddChildNode(root, bean_scn_light_node());

    SCNView_SetScene(scene_view, scene);
    bean_scn_view_set_point_of_view(scene_view, camera_node);
    SCNView_SetBackgroundColor(
        scene_view,
        NSColor_ColorWithCalibratedWhite_Alpha(0.12, 1.0));
    SCNView_SetAllowsCameraControl(scene_view, YES);
    SCNView_SetRendersContinuously(scene_view, YES);

    {
        SCNVector3 axis = { 0.0f, 1.0f, 0.0f };
        SCNAction rotate = SCNAction_RotateByAngle_AroundAxis_Duration((CGFloat)(2.0 * M_PI), axis, 4.0);
        SCNAction spin = SCNAction_RepeatActionForever(rotate);
        bean_scn_node_run_action(box_node, spin);
    }

    SCNView_Play(scene_view, NULL);

    printf("SceneKit: rotating blue box. Drag to orbit the camera.\n");
    bean_example_run(window);

    bean_autorelease_pool_pop(pool);
    return 0;
}
