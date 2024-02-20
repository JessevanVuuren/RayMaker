#include <math.h>
#include <stdbool.h>
#include <stdio.h>


#include "raylib.h"
#include "raymath.h"
#include "rcamera.h"

#define RAYGUI_IMPLEMENTATION
#define STB_DS_IMPLEMENTATION
#include "components.h"



#define HEIGHT 800
#define WIDTH 1200

#define CAM_ROT_SPEED 0.007
#define CAM_MOVE_SPEED .05
#define ZOOM_SPEED 1

#define SUPPORT_MODULE_RMODELS

Vector3 camera_start_pos = {20, 20, -20};

Vector3 origin = {0};

int main() {
    Camera3D cam = {.fovy = 90, .position = camera_start_pos, .target = origin, .projection = CAMERA_PERSPECTIVE, .up = (Vector3){0, 1, 0}};

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    InitWindow(WIDTH, HEIGHT, "RayMaker");
    SetExitKey(0);
    GuiLoadStyle("resources/styles/dark/style_dark.rgs");

    Font segoe_font = LoadFontEx("resources/segoe-ui.ttf", 200, 0, 0);

    Button *buttons = NULL;
    InputText matrix_input[9] = {0};

    int ui_bounding_size = 3;
    Rectangle *ui_bounding_box = update_ui_box(WIDTH, HEIGHT, ui_bounding_size);

    int current_width = WIDTH;
    int current_height = HEIGHT;

    arrput(buttons, load_button("resources/icons/move.png", "move", 15, 15));
    arrput(buttons, load_button("resources/icons/rotate.png", "rotate", 15, 65));
    arrput(buttons, load_button("resources/icons/scale.png", "scale", 15, 115));
    buttons[0].pressed = true;
    int selected_button_index = 0;

    // load_object(&objects, "resources/models/cars.obj", "resources/models/cars.png", "cars");
    // load_object(&objects, "resources/models/cars.obj", "resources/models/cars.png", "cars2");
    // load_object(&objects, "resources/models/church.obj", "resources/models/church.png", "church");
    // load_object("resources/models/church.obj", "resources/models/church.png", "church1");
    // objects[1].model.transform = MatrixTranslate(20, 0, 0);

    Selected selected = {0};

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

            ui_bounding_box = update_ui_box(current_width, current_height, ui_bounding_size);
        }

        float dist = GetMouseWheelMove();
        CameraMoveToTarget(&cam, -dist * ZOOM_SPEED);

        Vector3 direction = Vector3Subtract(cam.target, cam.position);
        direction = Vector3Normalize(direction);

        Vector3 newTarget = Vector3Add(cam.position, Vector3Scale(direction, 1.1));
        cam.target = newTarget;

        if (!mouse_is_in_ui_elements(ui_bounding_box, ui_bounding_size)) {
            Vector2 mouseDelta = GetMouseDelta();


            if ((IsMouseButtonDown(MOUSE_BUTTON_LEFT) && IsKeyDown(KEY_LEFT_CONTROL)) || IsMouseButtonDown(MOUSE_BUTTON_MIDDLE)) {

                CameraYaw(&cam, -mouseDelta.x * CAM_ROT_SPEED, true);
                CameraPitch(&cam, -mouseDelta.y * CAM_ROT_SPEED, true, true, false);
            }



            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
                callJSFunction(-GetMouseDelta().x);
                CameraMoveRight(&cam, -GetMouseDelta().x * CAM_MOVE_SPEED, false);
                CameraMoveUp(&cam, GetMouseDelta().y * CAM_MOVE_SPEED);
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsKeyDown(KEY_LEFT_CONTROL)) {
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
                    if (!objects[i].is_collection) {
                        RayCollision box = GetRayCollisionBox(ray, GetModelBoundingBox(objects[i].model));
                        // RayCollision box = GetRayCollisionMesh(ray, objects[i].model.meshes[0], objects[i].model.transform);
                        if (box.hit) {
                            dit_not_hit = false;
                            update_selected(&selected, objects[i], i, true);
                        }
                        if (dit_not_hit && !xyz_control.x.ray.hit && !xyz_control.y.ray.hit && !xyz_control.z.ray.hit) {
                            selected.is_selected = false;
                        }
                    }
                }
            }
        }


        if (IsKeyPressed(KEY_ESCAPE)) {
            selected.is_selected = false;
        }

        if (IsKeyPressed(KEY_R)) {
            cam.position = camera_start_pos;
            cam.up = (Vector3){0, 1, 0};
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
            if (arrlen(objects) > 0) {
                update_selected(&selected, objects[selected.index], selected.index, selected.is_selected);
                xyz_control.x.ray.hit = false;
                xyz_control.y.ray.hit = false;
                xyz_control.z.ray.hit = false;
            }
        }

        // clang-format off
        BeginTextureMode(world_layer);
            ClearBackground(GetColor(0x181818FF));
            BeginMode3D(cam);
                draw_graph(100, 101);
                if (xyz_control.x.ray.hit) {
                    Vector2 camera_pos = {cam.position.z, cam.position.y};
                    Mesh cube = xyz_control.hidden_box;

                    Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.x, control_mode);
                    move_collection(objects, selected, new_position);

                    objects[selected.index].model.transform = new_position;
                }

                if (xyz_control.y.ray.hit) {
                    Vector2 camera_pos = {cam.position.x, cam.position.z};
                    Mesh cube = xyz_control.hidden_box;

                    Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.y, control_mode);
                    move_collection(objects, selected, new_position);

                    objects[selected.index].model.transform = new_position;                
                }

                if (xyz_control.z.ray.hit) {
                    Vector2 camera_pos = {cam.position.y, cam.position.x};
                    Mesh cube = xyz_control.hidden_box;

                    Matrix new_position = move_object(cam, &selected, cube, camera_pos, xyz_control.z, control_mode);
                    move_collection(objects, selected, new_position);
                    
                    objects[selected.index].model.transform = new_position;
                }

                Vector3 edit_pos = origin;
                if (arrlen(objects) > 0) draw_models(objects, selected);
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
            for (int i = 0; i < ui_bounding_size; i++)
            {
                Rectangle box = ui_bounding_box[i];
                DrawRectangle(box.x,box.y, box.width, box.height, GetColor(0x282828FF));
            }
             
            render_buttons(buttons, arrlen(buttons));
            component_list(objects, &selected, current_width, current_height, segoe_font);

        EndTextureMode();

        BeginDrawing();
            DrawTextureRec(world_layer.texture, (Rectangle){0, 0, world_layer.texture.width, -world_layer.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(xyz_layer.texture, (Rectangle){0, 0, xyz_layer.texture.width, -xyz_layer.texture.height}, (Vector2){0, 0}, WHITE);
            DrawTextureRec(ui_layer.texture, (Rectangle){0, 0, ui_layer.texture.width, -ui_layer.texture.height}, (Vector2){0, 0}, WHITE);
            if (arrlen(objects) > 0) matrix_display(selected, objects, matrix_input, segoe_font);
        EndDrawing();
        // clang-format on
    }
    CloseWindow();


    return 0;
}