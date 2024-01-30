#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "extraray.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define HEIGHT 800
#define WIDTH 1200

#define CAMERA_SPEED 0.01
#define ZOOM_SPEED 1

#define SCENE_SIZE 100
#define SCENE_DETAIL 101
#define SCENE_GRID_SPACE 1
#define EDIT_TOOLS_SCALE 4

enum EditMode { MOVE,
                SCALE,
                ROTATE,
                NONE

};


typedef struct {
    int id;
    float angle;
    Model model;
    Vector3 pos;
    Vector3 scale;
    Texture2D texture;
    Vector3 rotation_axis;
    Vector3 current_rotation;
} Object;

typedef struct {
    int id;
    Mesh mesh;
    Matrix matrix;
    Material material;
} HitObject;


typedef struct {
    HitObject hit_box;
    HitObject rotation_box;
    RayCollision ray;
    Vector3 axis;
    Vector3 rotation_axis;
} AxisControl;

typedef struct {
    AxisControl x;
    AxisControl y;
    AxisControl z;
    Mesh hidden_box;
    Mesh rotation_plane;
} XYZcontrol;

typedef struct {
    Object object;
    bool is_selected;
    Vector3 pos;
    int index;
} Selected;

Vector3 camera_start_pos = {10, 10, -10};
Vector3 camera_start_up = {0, 1, 0};
Vector3 origin = {0};

void draw_graph() {

    for (int i = 0; i < SCENE_DETAIL; i++) {
        float step = SCENE_SIZE - (2 * SCENE_SIZE * i) / (float)(SCENE_DETAIL - 1);
        float point_one = -sqrtf(powf(SCENE_SIZE, 2) - powf(step, 2));
        float point_two = sqrtf(powf(SCENE_SIZE, 2) - powf(step, 2));

        DrawLine3D((Vector3){point_one, 0, step}, (Vector3){point_two, 0, step}, GetColor(0x454545FF));
        DrawLine3D((Vector3){step, 0, point_one}, (Vector3){step, 0, point_two}, GetColor(0x454545FF));
    }
}

XYZcontrol init_XYZ_controls() {
    XYZcontrol xyz;
    xyz.hidden_box = GenMeshCube(20000, 20000, 20000);

    xyz.x.ray.hit = false;
    xyz.y.ray.hit = false;
    xyz.z.ray.hit = false;

    xyz.x.axis = (Vector3){1, 0, 0};
    xyz.y.axis = (Vector3){0, 1, 0};
    xyz.z.axis = (Vector3){0, 0, 1};

    xyz.x.rotation_axis = (Vector3){0, 1, 0};
    xyz.y.rotation_axis = (Vector3){0, 0, 1};
    xyz.z.rotation_axis = (Vector3){1, 0, 0};

    xyz.x.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.y.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.z.hit_box.mesh = GenMeshPlane(.6, 7.5, 1, 1);

    xyz.x.rotation_box.material = LoadMaterialDefault();
    xyz.x.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF0000FF);

    xyz.y.rotation_box.material = LoadMaterialDefault();
    xyz.y.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00FF00FF);

    xyz.z.rotation_box.material = LoadMaterialDefault();
    xyz.z.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x0000FFFF);

    xyz.x.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.y.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.z.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);

    return xyz;
}

void draw_xyz_control(Vector3 target, enum EditMode mode, Camera3D cam, XYZcontrol *xyz) {
    Vector3 end_pos_x_axis = Vector3Add(target, (Vector3){7.5f, 0, 0});
    Vector3 end_pos_y_axis = Vector3Add(target, (Vector3){0, 7.5f, 0});
    Vector3 end_pos_z_axis = Vector3Add(target, (Vector3){0, 0, -7.5f});

    if (mode == MOVE) {
        DrawCylinderEx(end_pos_x_axis, (Vector3){end_pos_x_axis.x + 1, end_pos_x_axis.y, end_pos_x_axis.z}, .25, 0, 30, GetColor(0xFF0000FF));
        DrawCylinderEx(end_pos_y_axis, (Vector3){end_pos_y_axis.x, end_pos_y_axis.y + 1, end_pos_y_axis.z}, .25, 0, 30, GetColor(0x00FF00FF));
        DrawCylinderEx(end_pos_z_axis, (Vector3){end_pos_z_axis.x, end_pos_z_axis.y, end_pos_z_axis.z - 1}, .25, 0, 30, GetColor(0x0000FFFF));
    };
    if (mode == SCALE) {
        DrawCube(end_pos_x_axis, .65, .65, .65, GetColor(0xFF0000FF));
        DrawCube(end_pos_y_axis, .65, .65, .65, GetColor(0x00FF00FF));
        DrawCube(end_pos_z_axis, .65, .65, .65, GetColor(0x0000FFFF));
    };
    if (mode == ROTATE) {
        DrawMesh(xyz->x.rotation_box.mesh, xyz->x.rotation_box.material, xyz->x.rotation_box.matrix);
        DrawMesh(xyz->y.rotation_box.mesh, xyz->y.rotation_box.material, xyz->y.rotation_box.matrix);
        DrawMesh(xyz->z.rotation_box.mesh, xyz->z.rotation_box.material, xyz->z.rotation_box.matrix);
    };

    DrawLine3D(target, end_pos_x_axis, GetColor(0xFF0000FF));
    DrawLine3D(target, end_pos_y_axis, GetColor(0x00FF00FF));
    DrawLine3D(target, end_pos_z_axis, GetColor(0x0000FFFF));


    float angle_x = atan2f(cam.position.z, cam.position.y);
    float angle_z = atan2f(cam.position.x, cam.position.y);
    float angle_y = atan2f(cam.position.z, cam.position.x);

    end_pos_x_axis.x -= 7.5f * .5f;
    end_pos_y_axis.y -= 7.5f * .5f;
    end_pos_z_axis.z += 7.5f * .5f;


    xyz->x.rotation_box.matrix = Vector3Translate(target);
    xyz->y.rotation_box.matrix = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), Vector3Translate(target));
    xyz->z.rotation_box.matrix = MatrixMultiply(MatrixRotateZ(90 * DEG2RAD), Vector3Translate(target));

    xyz->x.hit_box.matrix = MatrixMultiply(MatrixRotateX(angle_x), Vector3Translate(end_pos_x_axis));
    xyz->y.hit_box.matrix = MatrixMultiply(MatrixRotateY(-angle_y), Vector3Translate(end_pos_y_axis));
    xyz->z.hit_box.matrix = MatrixMultiply(MatrixRotateZ(-angle_z), Vector3Translate(end_pos_z_axis));
    xyz->y.hit_box.matrix = MatrixMultiply(MatrixRotateZ(-(90 * DEG2RAD)), xyz->y.hit_box.matrix);
}

Selected update_selected(Object object, int index, bool is_selected, Selected curr_selected) {
    Selected selected;
    selected.object = object;
    selected.object.angle += object.angle;
    selected.object.rotation_axis = object.rotation_axis;
    selected.pos = getMatrixPosition(object.model.transform);
    selected.index = index;
    selected.is_selected = is_selected;
    return selected;
}

void update_object(Selected selected, Object *object) {
    Vector3 angle = Vector3Multiply(object->rotation_axis, makeVector3(object->angle));
    object->current_rotation = Vector3Add(object->current_rotation, angle);
    object->angle = 0;
}

Matrix move_object(Camera cam, Selected *selected, Mesh cube, Vector2 camera_pos, AxisControl xyz, enum EditMode mode, Object *object) {
    Selected selected_object = *selected;
    Ray ray = GetMouseRay(GetMousePosition(), cam);

    float angle = atan2f(camera_pos.x, camera_pos.y);
    float scale = 10000 / sqrtf(powf(camera_pos.x, 2) + powf(camera_pos.y, 2));

    Vector3 inverse_axis = Vector3Scale(Vector3AddValue(xyz.axis, -1), -1);
    Vector3 opposite_cam = Vector3Scale(cam.position, -scale);
    Vector3 current_axis = Vector3Add(Vector3Multiply(opposite_cam, inverse_axis), selected_object.pos);


    Matrix translation = MatrixTranslate(current_axis.x, current_axis.y, current_axis.z);
    Matrix rotation = MatrixRotate(xyz.axis, angle);
    Matrix matrix = MatrixMultiply(rotation, translation);

    if (mode == ROTATE) {
        Vector3 hidden_rotation_cube = Vector3Multiply((Vector3){-10000, -10000, 10000}, xyz.rotation_axis);
        Vector3 ww = Vector3Add(selected_object.pos, hidden_rotation_cube);
        matrix = Vector3Translate(ww);
    }

    Vector3 hit_point = GetRayCollisionMesh(ray, cube, matrix).point;
    Vector3 hit_point_offset = Vector3Subtract(hit_point, Vector3Multiply(xyz.axis, xyz.ray.point));
    Vector3 offset_current_axis = Vector3Multiply(xyz.axis, hit_point_offset);

    Matrix manipulated_matrix;
    if (mode == MOVE) {
        Vector3 new_position = Vector3Add(selected_object.pos, offset_current_axis);
        manipulated_matrix = MatrixTranslate(new_position.x, new_position.y, new_position.z);
    }
    if (mode == ROTATE) {
        float rotate_angle = Vector3Angle(xyz.ray.point, hit_point);
        Vector3 cross_product = Vector3CrossProduct(xyz.ray.point, hit_point);
        if (getAxisValue(xyz.rotation_axis, cross_product) < 0) rotate_angle *= -1;
        Matrix rotate = MatrixRotate(xyz.rotation_axis, rotate_angle);
        manipulated_matrix = MatrixMultiply(selected_object.object.model.transform, rotate);

        object->angle = rotate_angle;
        object->rotation_axis = xyz.rotation_axis;
    }

    if (mode == SCALE) {
        offset_current_axis.z = -offset_current_axis.z;
        Vector3 current_axis_scale = Vector3Multiply(offset_current_axis, xyz.axis);
        Vector3 add_base_one = Vector3AddValue(current_axis_scale, 1);
        Matrix matrix_scaled_up = MatrixScale(add_base_one.x, add_base_one.y, add_base_one.z);
        manipulated_matrix = MatrixMultiply(matrix_scaled_up, selected_object.object.model.transform);
    }
    return manipulated_matrix;
}


void draw_model(Object o, Selected selected) {
    DrawModel(o.model, o.pos, 1, WHITE);
    if (selected.object.id == o.id) {
        Model normalized_model = LoadModelFromMesh(o.model.meshes[0]);
            // printV(o.current_rotation);

        Matrix rot = RotationMatrixFromEuler(o.current_rotation.x, o.current_rotation.y, o.current_rotation.z);
        Vector3 axis;
        float angle;
        MatrixToAxisAngle(rot, &axis, &angle);
        printV(axis);
        DrawModelWiresEx(normalized_model, o.pos, axis, angle * RAD2DEG, (Vector3){1, 1, 1}, WHITE);

        // if (o.rotation_axis.x == 1) {
        // }
        // if (o.rotation_axis.y == 1) {
        //     DrawModelWiresEx(normalized_model, o.pos, o.rotation_axis, ( o.current_rotation.y) * RAD2DEG, (Vector3){1, 1, 1}, WHITE);
        // }
        // if (o.rotation_axis.z == 1) {
        //     DrawModelWiresEx(normalized_model, o.pos, o.rotation_axis, ( o.current_rotation.z) * RAD2DEG, (Vector3){1, 1, 1}, WHITE);
        // }
    }
}

int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);

    Object *objects = NULL;

    XYZcontrol xyz_control = init_XYZ_controls();

    Object model1;
    model1.id = 1;
    model1.scale = Vector3One();
    model1.pos = (Vector3){0, 0, 0};
    model1.angle = 0;
    model1.model = LoadModel("resources/models/church.obj");
    model1.texture = LoadTexture("resources/models/church_diffuse.png");
    model1.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = model1.texture;

    arrput(objects, model1);

    Selected selected = {0};

    RenderTexture2D world_render = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D xyz_render = LoadRenderTexture(WIDTH, HEIGHT);

    enum EditMode control_mode = ROTATE;

    while (!WindowShouldClose()) {

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL)) {
            Vector2 mouseDelta = GetMouseDelta();

            CameraYaw(&cam, -mouseDelta.x * CAMERA_SPEED, true);
            CameraPitch(&cam, -mouseDelta.y * CAMERA_SPEED, true, true, false);
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            selected.is_selected = false;
        }

        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = camera_start_up;
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);

            if (control_mode != ROTATE) {
                xyz_control.x.ray = GetRayCollisionMesh(ray, xyz_control.x.hit_box.mesh, xyz_control.x.hit_box.matrix);
                xyz_control.y.ray = GetRayCollisionMesh(ray, xyz_control.y.hit_box.mesh, xyz_control.y.hit_box.matrix);
                xyz_control.z.ray = GetRayCollisionMesh(ray, xyz_control.z.hit_box.mesh, xyz_control.z.hit_box.matrix);
            } else {
                xyz_control.x.ray = GetRayCollisionMesh(ray, xyz_control.x.rotation_box.mesh, xyz_control.x.rotation_box.matrix);
                xyz_control.y.ray = GetRayCollisionMesh(ray, xyz_control.y.rotation_box.mesh, xyz_control.y.rotation_box.matrix);
                xyz_control.z.ray = GetRayCollisionMesh(ray, xyz_control.z.rotation_box.mesh, xyz_control.z.rotation_box.matrix);
            }

            for (int i = 0; i < arrlen(objects); i++) {
                RayCollision box = GetRayCollisionBox(ray, GetModelBoundingBox(objects[i].model));
                if (box.hit) selected = update_selected(objects[i], i, true, selected);
            }
        }

        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            selected = update_selected(objects[selected.index], selected.index, selected.is_selected, selected);
            update_object(selected, &objects[selected.index]);
            xyz_control.x.ray.hit = false;
            xyz_control.y.ray.hit = false;
            xyz_control.z.ray.hit = false;
        }

        BeginTextureMode(world_render);
        ClearBackground(GetColor(0x181818FF));

        BeginMode3D(cam);
        draw_graph();

        Vector3 edit_pos = origin;
        enum EditMode edit = NONE;

        if (xyz_control.x.ray.hit) {
            Vector2 camera_pos = {cam.position.z, cam.position.y};
            Mesh cube = xyz_control.hidden_box;

            Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.x, control_mode, &objects[selected.index]);
            objects[selected.index].model.transform = new_position;
        }

        if (xyz_control.y.ray.hit) {
            Vector2 camera_pos = {cam.position.x, cam.position.z};
            Mesh cube = xyz_control.hidden_box;

            Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.y, control_mode, &objects[selected.index]);
            objects[selected.index].model.transform = new_position;
        }

        if (xyz_control.z.ray.hit) {
            Vector2 camera_pos = {cam.position.y, cam.position.x};
            Mesh cube = xyz_control.hidden_box;

            Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.z, control_mode, &objects[selected.index]);
            objects[selected.index].model.transform = new_position;
        }

        for (int i = 0; i < arrlen(objects); i++) {
            draw_model(objects[i], selected);
        }


        if (selected.is_selected) {
            edit_pos = getMatrixPosition(objects[selected.index].model.transform);
            edit = control_mode;
        }
        EndMode3D();
        DrawFPS(10, 10);
        EndTextureMode();

        BeginTextureMode(xyz_render);
        BeginMode3D(cam);
        ClearBackground(GetColor(0x00000000));

        draw_xyz_control(edit_pos, edit, cam, &xyz_control);
        EndMode3D();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));
        DrawTextureRec(world_render.texture, (Rectangle){0, 0, world_render.texture.width, -world_render.texture.height}, (Vector2){0, 0}, WHITE);
        DrawTextureRec(xyz_render.texture, (Rectangle){0, 0, xyz_render.texture.width, -xyz_render.texture.height}, (Vector2){0, 0}, WHITE);
        EndDrawing();
    }

    UnloadRenderTexture(world_render);
    UnloadRenderTexture(xyz_render);
    CloseWindow();
}