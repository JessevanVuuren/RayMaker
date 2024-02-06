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


int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = camera_start_up};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetTargetFPS(60);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);

    Object *objects = NULL;
    Button *buttons = NULL;

    int current_width = WIDTH;
    int current_height = HEIGHT;
    Rectangle view_port = {60, 0, WIDTH - 360, HEIGHT};


    arrput(buttons, load_button("resources/icons/move.png", "move", 15, 15));
    arrput(buttons, load_button("resources/icons/rotate.png", "rotate", 15, 65));
    arrput(buttons, load_button("resources/icons/scale.png", "scale", 15, 115));
    buttons[0].pressed = true;
    int selected_button_index = 0;

    arrput(objects, load_object("resources/models/church.obj", "resources/models/church_diffuse.png", 1));

    Selected selected = {0};

    EditMode control_mode = MOVE;

    XYZcontrol xyz_control = init_XYZ_controls();
    RenderTexture2D background = LoadRenderTexture(current_width, current_height);
    RenderTexture2D xyz_render = LoadRenderTexture(view_port.width, view_port.height);
    RenderTexture2D world_render = LoadRenderTexture(view_port.width, view_port.height);
    while (!WindowShouldClose()) {

        if (IsWindowResized()) {
            current_width = GetScreenWidth();
            current_height = GetScreenHeight();

            view_port.width = current_width - 360;
            view_port.height = current_height;

            background = LoadRenderTexture(current_width, current_height);
            xyz_render = LoadRenderTexture(view_port.width, view_port.height);
            world_render = LoadRenderTexture(view_port.width, view_port.height);
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

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL) && GetMousePosition().x > 60) {
            Ray ray = GetMouseRay(GetMousePosition(), cam);
            bool dit_not_hit = true;

            if (selected.is_selected) {
                if (control_mode != ROTATE) {
                    printf("%d\n", xyz_control.x.ray.hit);
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

        BeginTextureMode(background);
            ClearBackground(GetColor(0x282828FF));
            render_buttons(buttons, arrlen(buttons));
            component_list(buttons, arrlen(buttons));
        EndTextureMode();

        BeginTextureMode(world_render);
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
            DrawFPS(10, current_height - 25);
        EndTextureMode();

        BeginTextureMode(xyz_render);
            BeginMode3D(cam);
                ClearBackground(GetColor(0x00000000));
                if (selected.is_selected) draw_xyz_control(edit_pos, control_mode, cam, &xyz_control);
            EndMode3D();
        EndTextureMode();

        BeginDrawing();
            DrawTextureRec(background.texture, (Rectangle){0, 0, background.texture.width + 200, -background.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(world_render.texture, (Rectangle){0, 0, world_render.texture.width, -world_render.texture.height}, (Vector2){view_port.x, view_port.y}, WHITE);
            DrawTextureRec(xyz_render.texture, (Rectangle){0, 0, xyz_render.texture.width, -xyz_render.texture.height}, (Vector2){view_port.x, view_port.y}, WHITE);
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