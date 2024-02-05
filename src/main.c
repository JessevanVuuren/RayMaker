#include <math.h>
#include <stdbool.h>
#include <stdio.h>

#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "components.h"

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

typedef enum { MOVE,
               SCALE,
               ROTATE,
               NONE

} EditMode;


typedef struct {
    int id;
    Model model;
    Texture2D texture;
} Object;



typedef struct {
    int index;
    Vector3 pos;
    Object object;
    bool is_selected;
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



void draw_xyz_control(Vector3 target, EditMode mode, Camera3D cam, XYZcontrol *xyz) {
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
    selected.pos = getMatrixPosition(object.model.transform);
    selected.index = index;
    selected.is_selected = is_selected;
    return selected;
}


Matrix move_object(Camera cam, Selected *selected, Mesh cube, Vector2 camera_pos, AxisControl xyz, EditMode mode, Object *object) {
    Selected selected_object = *selected;
    Object tt = *object;
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

    // Material mat = LoadMaterialDefault();
    // mat.maps[MATERIAL_MAP_DIFFUSE].color = PURPLE;
    // DrawMesh(cube, mat, matrix);

    Vector3 hit_point = GetRayCollisionMesh(ray, cube, matrix).point;
    Vector3 hit_point_offset = Vector3Subtract(hit_point, Vector3Multiply(xyz.axis, xyz.ray.point));
    Vector3 offset_current_axis = Vector3Multiply(xyz.axis, hit_point_offset);

    Matrix manipulated_matrix;
    if (mode == MOVE) {
        manipulated_matrix = MatrixMultiply(selected_object.object.model.transform, Vector3Translate(offset_current_axis));
    }
    if (mode == ROTATE) {
        Vector3 base_point = Vector3Subtract(xyz.ray.point, selected_object.pos);
        Vector3 drag_point = Vector3Subtract(hit_point, selected_object.pos);

        float rotate_angle = Vector3Angle(base_point, drag_point);
        Vector3 cross_product = Vector3CrossProduct(base_point, drag_point);

        if (getAxisValue(xyz.rotation_axis, cross_product) < 0) rotate_angle *= -1;

        Matrix rotate = MatrixRotate(xyz.rotation_axis, rotate_angle);
        Matrix rotate_at_origin = MatrixMultiply(MatrixIdentity(), rotate);
        Matrix rotate_to_position = MatrixMultiply(selected_object.object.model.transform, rotate_at_origin);

        manipulated_matrix = selected_object.object.model.transform;
        CopyRotationMatrix(&rotate_to_position, &manipulated_matrix);
    }

    if (mode == SCALE) {
        offset_current_axis.z = -offset_current_axis.z;
        Vector3 add_base_one = Vector3AddValue(offset_current_axis, 1);
        Matrix matrix_scaled_up = MatrixScale(add_base_one.x, add_base_one.y, add_base_one.z);
        manipulated_matrix = MatrixMultiply(selected_object.object.model.transform, matrix_scaled_up);
    }
    return manipulated_matrix;
}

void draw_model(Object o, Selected selected) {
    DrawModel(o.model, Vector3Zero(), 1, WHITE);
    if (selected.object.id == o.id && selected.is_selected) {
        Vector3 axis;
        float angle;
        MatrixToAxisAngle(o.model.transform, &axis, &angle);

        for (int i = 0; i < o.model.meshCount; i++) {
            Model normalized_model = LoadModelFromMesh(o.model.meshes[i]);
            DrawModelWiresEx(normalized_model, getMatrixPosition(o.model.transform), axis, angle * RAD2DEG, (Vector3){1, 1, 1}, WHITE);
        }
    }
}

Object load_object(char *model, char *texture, int id) {
    Object object;
    object.id = id;
    object.model = LoadModel(model);
    object.texture = LoadTexture(texture);
    object.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = object.texture;
    return object;
}

void button_pressed(int button_index, EditMode *edit) {
    switch (button_index) {
        case 0:
            *edit = MOVE;
            break;

        case 1:
            *edit = ROTATE;
            break;

        case 2:
            *edit = SCALE;
            break;

        default:
            break;
    }
}

int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);

    Object *objects = NULL;
    Button *buttons = NULL;


    arrput(buttons, load_button("resources/icons/move.png", "move", 15, 15));
    arrput(buttons, load_button("resources/icons/rotate.png", "rotate", 15, 65));
    arrput(buttons, load_button("resources/icons/scale.png", "scale", 15, 115));
    buttons[0].pressed = true;
    int selected_button_index = 0;

    arrput(objects, load_object("resources/models/church.obj", "resources/models/church_diffuse.png", 1));

    Selected selected = {0};

    EditMode control_mode = SCALE;

    XYZcontrol xyz_control = init_XYZ_controls();
    RenderTexture2D xyz_render = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D world_render = LoadRenderTexture(WIDTH, HEIGHT);
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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL) && GetMousePosition().x > 60) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);
            bool dit_not_hit = true;

            if (selected.is_selected) {
                if (control_mode != ROTATE) {
                    xyz_control.x.ray = GetRayCollisionMesh(ray, xyz_control.x.hit_box.mesh, xyz_control.x.hit_box.matrix);
                    xyz_control.y.ray = GetRayCollisionMesh(ray, xyz_control.y.hit_box.mesh, xyz_control.y.hit_box.matrix);
                    xyz_control.z.ray = GetRayCollisionMesh(ray, xyz_control.z.hit_box.mesh, xyz_control.z.hit_box.matrix);
                } else {
                    xyz_control.x.ray = GetRayCollisionMesh(ray, xyz_control.x.rotation_box.mesh, xyz_control.x.rotation_box.matrix);
                    xyz_control.y.ray = GetRayCollisionMesh(ray, xyz_control.y.rotation_box.mesh, xyz_control.y.rotation_box.matrix);
                    xyz_control.z.ray = GetRayCollisionMesh(ray, xyz_control.z.rotation_box.mesh, xyz_control.z.rotation_box.matrix);
                }
            }

            for (int i = 0; i < arrlen(objects); i++) {

                RayCollision box = GetRayCollisionBox(ray, GetModelBoundingBox(objects[i].model));
                // RayCollision box = GetRayCollisionMesh(ray, objects[i].model.meshes[0], objects[i].model.transform);
                if (box.hit) {
                    dit_not_hit = false;
                    selected = update_selected(objects[i], i, true, selected);
                }
                if (dit_not_hit && !xyz_control.x.ray.hit && !xyz_control.y.ray.hit && !xyz_control.z.ray.hit) {
                    selected.is_selected = false;
                }
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 mouse_pos = GetMousePosition();
            if (mouse_pos.x < 60) {

                for (int i = 0; i < arrlen(buttons); i++) {
                    if (CheckCollisionPointRec(mouse_pos, buttons[i].rec)) {
                        buttons[i].pressed = true;
                        selected_button_index = i;
                    }
                }
                for (int i = 0; i < arrlen(buttons); i++)
                    if (i != selected_button_index) buttons[i].pressed = false;

                button_pressed(selected_button_index, &control_mode);
            }
        }



        if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
            selected = update_selected(objects[selected.index], selected.index, selected.is_selected, selected);
            xyz_control.x.ray.hit = false;
            xyz_control.y.ray.hit = false;
            xyz_control.z.ray.hit = false;
        }

        // clang-format off
        BeginTextureMode(world_render);
            ClearBackground(GetColor(0x181818FF));
            BeginMode3D(cam);
                draw_graph();

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

                Vector3 edit_pos = origin;

                for (int i = 0; i < arrlen(objects); i++) draw_model(objects[i], selected);
                if (selected.is_selected) edit_pos = getMatrixPosition(objects[selected.index].model.transform);
                
            EndMode3D();
            DrawFPS(70, HEIGHT - 25);
        EndTextureMode();

        BeginTextureMode(xyz_render);
            BeginMode3D(cam);
                ClearBackground(GetColor(0x00000000));
                if (selected.is_selected) draw_xyz_control(edit_pos, control_mode, cam, &xyz_control);

            EndMode3D();
            DrawRectangle(0,0, 60, HEIGHT, GetColor(0x282828FF));
            render_buttons(buttons, arrlen(buttons));
        EndTextureMode();

        BeginDrawing();
            ClearBackground(GetColor(0x181818FF));
            DrawTextureRec(world_render.texture, (Rectangle){0, 0, world_render.texture.width, -world_render.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(xyz_render.texture, (Rectangle){0, 0, xyz_render.texture.width, -xyz_render.texture.height}, (Vector2){0, 0}, WHITE);
        EndDrawing();
        // clang-format on
    }

    for (int i = 0; i < arrlen(objects); i++) UnloadModel(objects[i].model);
    for (int i = 0; i < arrlen(objects); i++) UnloadTexture(objects[i].texture);

    for (int i = 0; i < arrlen(buttons); i++) UnloadImage(buttons[i].img);
    for (int i = 0; i < arrlen(buttons); i++) UnloadTexture(buttons[i].texture);

    UnloadMesh(xyz_control.hidden_box);
    UnloadMesh(xyz_control.x.hit_box.mesh);
    UnloadMesh(xyz_control.y.hit_box.mesh);
    UnloadMesh(xyz_control.z.hit_box.mesh);

    UnloadMesh(xyz_control.x.rotation_box.mesh);
    UnloadMaterial(xyz_control.x.rotation_box.material);
    UnloadMesh(xyz_control.y.rotation_box.mesh);
    UnloadMaterial(xyz_control.y.rotation_box.material);
    UnloadMesh(xyz_control.z.rotation_box.mesh);
    UnloadMaterial(xyz_control.z.rotation_box.material);


    UnloadRenderTexture(world_render);
    UnloadRenderTexture(xyz_render);
    CloseWindow();
}