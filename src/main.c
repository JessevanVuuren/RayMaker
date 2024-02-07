#include <math.h>
#include <stdbool.h>
#include <stdio.h>


#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#include "components.h"

#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define HEIGHT 800
#define WIDTH 1200

#define CAMERA_SPEED 0.01
#define ZOOM_SPEED 1


Vector3 camera_start_pos = {10, 10, -10};
Vector3 camera_start_up = {0, 1, 0};
Vector3 origin = {0};

bool mouse_is_in_ui_element(Rectangle box[], int size) {
    Vector2 pos = GetMousePosition();
    for (int i = 0; i < size; i++) {
        if (pos.x > box[i].x && pos.x < box[i].width && pos.y > box[i].y && pos.y < box[i].height) return true;
    }
    return false;
}


int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);


    Object *objects = NULL;
    Button *buttons = NULL;

    Rectangle ui_bounding_box[] = {
        {0, 0, 60, HEIGHT},
        {WIDTH - 300, 0, 300, 350}};
    int ui_bounding_box_size = sizeof(ui_bounding_box) / sizeof(ui_bounding_box[0]);

    int current_width = WIDTH;
    int current_height = HEIGHT;

    arrput(buttons, load_button("resources/icons/move.png", "move", 15, 15));
    arrput(buttons, load_button("resources/icons/rotate.png", "rotate", 15, 65));
    arrput(buttons, load_button("resources/icons/scale.png", "scale", 15, 115));
    buttons[0].pressed = true;
    int selected_button_index = 0;

    arrput(objects, load_object("resources/models/church.obj", "resources/models/church_diffuse.png", 1, "church1"));

    Selected selected = {0};

    EditMode control_mode = MOVE;

    XYZcontrol xyz_control = init_XYZ_controls();

    RenderTexture2D ui_layer = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D xyz_layer = LoadRenderTexture(WIDTH, HEIGHT);
    RenderTexture2D world_layer = LoadRenderTexture(WIDTH, HEIGHT);
    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            current_width = GetScreenWidth();
            current_height = GetScreenHeight();

            ui_layer = LoadRenderTexture(current_width, current_height);
            xyz_layer = LoadRenderTexture(current_width, current_height);
            world_layer = LoadRenderTexture(current_width, current_height);
        }

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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL)) {
            if (mouse_is_in_ui_element(ui_bounding_box, ui_bounding_box_size)) {
                printf("in ui\n");
            };

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
        BeginTextureMode(world_layer);
            ClearBackground(GetColor(0x181818FF));
            BeginMode3D(cam);
                draw_graph(100, 101);
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
            DrawFPS(70, current_height - 25);
        EndTextureMode();

        BeginTextureMode(xyz_layer);
            BeginMode3D(cam);
                ClearBackground(GetColor(0x00000000));
                if (selected.is_selected) draw_xyz_control(edit_pos, control_mode, cam, &xyz_control);
            EndMode3D();
        EndTextureMode();

        BeginTextureMode(ui_layer);
            for (int i = 0; i < ui_bounding_box_size; i++)
            {
                Rectangle box = ui_bounding_box[i];
                DrawRectangle(box.x,box.y, box.width, box.height, GetColor(0x282828FF));
            }
             
            render_buttons(buttons, arrlen(buttons));
            component_list(objects, arrlen(objects), current_width, current_height);
        EndTextureMode();

        BeginDrawing();
            DrawTextureRec(world_layer.texture, (Rectangle){0, 0, world_layer.texture.width, -world_layer.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(xyz_layer.texture, (Rectangle){0, 0, xyz_layer.texture.width, -xyz_layer.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(ui_layer.texture, (Rectangle){0, 0, ui_layer.texture.width, -ui_layer.texture.height}, (Vector2){0, 0}, WHITE);
        EndDrawing();
        // clang-format on
    }

    for (int i = 0; i < arrlen(objects); i++)
        UnloadModel(objects[i].model);
    for (int i = 0; i < arrlen(objects); i++)
        UnloadTexture(objects[i].texture);

    for (int i = 0; i < arrlen(buttons); i++)
        UnloadImage(buttons[i].img);
    for (int i = 0; i < arrlen(buttons); i++)
        UnloadTexture(buttons[i].texture);

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


    UnloadRenderTexture(ui_layer);
    UnloadRenderTexture(world_layer);
    UnloadRenderTexture(xyz_layer);
    CloseWindow();

    return 0;
}