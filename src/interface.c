#include "components.h"


Button load_button(char *img_path, char *name, int x, int y) {
    Button button;
    button.name = name;
    button.pressed = false;
    button.img_path = img_path;

    button.pos.x = x;
    button.pos.y = y;

    Image img = LoadImage(img_path);

    button.rec.x = x - 5;
    button.rec.y = y - 5;
    button.rec.width = img.width + 10;
    button.rec.height = img.height + 10;

    button.texture = LoadTextureFromImage(img);
    UnloadImage(img);
    return button;
}

void render_buttons(Button *buttons, int size) {
    for (int i = 0; i < size; i++) {
        Button b = buttons[i];
        Color color;
        if (b.pressed) color = GetColor(0xFFDD33FF);
        if (!b.pressed) color = GetColor(0xFFFFFFFF);

        DrawRectangleLines(b.rec.x, b.rec.y, b.rec.width, b.rec.height, color);
        DrawTextureEx(b.texture, b.pos, 0, 1, color);
    }
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

bool mouse_is_in_ui_elements(Rectangle box[], int size) {
    Vector2 pos = GetMousePosition();
    for (int i = 0; i < size; i++) {
        if (pos.x > box[i].x && pos.x < box[i].x + box[i].width && pos.y > box[i].y && pos.y < box[i].y + box[i].height) return true;
    }
    return false;
}

Rectangle *update_ui_box(int new_width, int new_height, int size) {
    Rectangle *ui_box_list = malloc(size * sizeof(Rectangle));
    ui_box_list[0] = (Rectangle){0, 0, 60, new_height};
    ui_box_list[1] = (Rectangle){new_width - 300, 0, 300, 340};
    ui_box_list[2] = (Rectangle){new_width - 300, 350, 300, 325};
    return ui_box_list;
}

bool mouse_in_rec(Vector2 mouse_pos, Rectangle rec) {
    if (mouse_pos.x > rec.x && mouse_pos.x < rec.x + rec.width && mouse_pos.y > rec.y && mouse_pos.y < rec.y + rec.height) {
        return true;
    }

    return false;
}

void display_item(Rectangle box, char *name, int object_index, Font font, Selected selected, int text_distance, bool is_collection) {
    Vector2 mouse = GetMousePosition();
    Color background_color = GetColor(0x181818FF);

    if (selected.object.id == object_index && selected.is_selected) {
        background_color = GetColor(0x383838FF);
    }

    if (mouse_in_rec(mouse, box)) {
        background_color = GetColor(0x484848FF);
    }
    DrawRectangleRec(box, background_color);

    if (is_collection) {
        box.x += 7.5;
        DrawLine(box.x, box.y, box.x, box.y + text_distance, GetColor(0x484848FF));
    }
    DrawTextEx(font, name, (Vector2){box.x + 5, box.y}, 20, 2, WHITE);
}

void item_pressed(Rectangle rec, Selected *selected, Object *objects, int index) {
    Vector2 mouse_pos = GetMousePosition();

    if (mouse_in_rec(mouse_pos, rec) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        update_selected(selected, objects[index], index, true);

        if (objects[index].is_collection) {
            update_child_matrix(selected, objects, index);
        }
    }
}

void component_list(Object *objects, Selected *selected, int width, int height, Font font) {
    int border_size = 10;
    int text_distance = 20;
    Vector2 box_size = {300, 340};
    Rectangle box = {width - box_size.x + border_size, border_size, box_size.x - border_size * 2, box_size.y - border_size * 2};

    DrawRectangle(box.x, box.y, box.width, box.height, GetColor(0x181818FF));
    // float dist = GetMouseWheelMove();
    BeginScissorMode(box.x, box.y, box.width, box.height);
    // box.y -= 50;

    for (int i = 0; i < arrlen(objects); i++) {
        Object o = objects[i];
        Rectangle rec = {box.x, box.y + i * text_distance, box.width, text_distance};
        display_item(rec, o.name, o.id, font, *selected, text_distance, o.is_part_of);

        item_pressed(rec, selected, objects, i);
    }
    EndScissorMode();
}

float input_value_matrix(Selected selected, InputText *input_text, Vector2 mouse_pos, Rectangle rect, float value, char *default_text) {
    int text_size = sizeof input_text->text;

    if (selected.is_selected) {
        if (input_text->is_selected == false) {
            input_text->is_selected = mouse_in_rec(mouse_pos, rect) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
            if (input_text->is_selected)
                snprintf(input_text->text, text_size, "%s", "\0");
            else
                snprintf(input_text->text, text_size, "%f", value);
        }
    } else {
        snprintf(input_text->text, text_size, "%s", default_text);
        input_text->is_selected = false;
    }

    GuiTextBox(rect, input_text->text, text_size, input_text->is_selected);

    float number;
    sscanf(input_text->text, "%f", &number);
    return number;
}

void apply_translation(Rectangle rect, InputText *in, Selected selected, Matrix *matrix, Vector3 new_pos, Object *objects) {
    rect.x += 170;
    rect.width = 90;

    if (GuiButton(rect, "Apply") && selected.is_selected) {
        matrix->m12 = new_pos.x;
        matrix->m13 = new_pos.y;
        matrix->m14 = new_pos.z;

        if (selected.object.is_collection) {
            move_collection(objects, selected, *matrix);
        }

        in->is_selected = false;
    }
}

void apply_scale(Rectangle rect, InputText *in, Selected selected, Matrix *matrix, Vector3 scale, Object *objects) {
    rect.x += 170;
    rect.width = 90;

    if (GuiButton(rect, "Apply") && selected.is_selected) {
        *matrix = set_matrix_scale(*matrix, scale);

        if (selected.object.is_collection) {
            move_collection(objects, selected, *matrix);
        }

        in->is_selected = false;
    }
}

void apply_rotation(Rectangle rect, InputText *in, Selected selected, Matrix *matrix, Vector3 axis_angle, Object *objects) {
    rect.x += 170;
    rect.width = 90;

    if (GuiButton(rect, "Apply") && selected.is_selected) {
        *matrix = set_matrix_rotation(*matrix, axis_angle);

        if (selected.object.is_collection) {
            move_collection(objects, selected, *matrix);
        }

        in->is_selected = false;
    }
}

void matrix_display(Selected selected, Object *objects, InputText *matrix_input, Font font) {
    Vector2 input_size = {160, 25};
    Vector2 mouse_pos = GetMousePosition();
    Rectangle rect = {.x = GetScreenWidth() - 280, .y = 370, .width = input_size.x, .height = input_size.y};
    DrawRectangle(GetScreenWidth() - 290, 360, 280, 305, GetColor(0x181818FF));

    Matrix matrix = objects[selected.index].model.transform;
    Quaternion quaternion = QuaternionFromMatrix(matrix);
    Vector3 rotation = QuaternionToEuler(quaternion);
    Vector3 scale = GetScaleFromMatrix(matrix);

    Matrix *object_matrix = &objects[selected.index].model.transform;


    float translate_X = input_value_matrix(selected, &matrix_input[0], mouse_pos, rect, matrix.m12, "X position");
    apply_translation(rect, &matrix_input[0], selected, object_matrix, (Vector3){translate_X, matrix.m13, matrix.m14}, objects);

    rect.y += 30;
    float translate_Y = input_value_matrix(selected, &matrix_input[1], mouse_pos, rect, matrix.m13, "Y position");
    apply_translation(rect, &matrix_input[1], selected, object_matrix, (Vector3){matrix.m12, translate_Y, matrix.m14}, objects);

    rect.y += 30;
    float translate_Z = input_value_matrix(selected, &matrix_input[2], mouse_pos, rect, matrix.m14, "Z position");
    apply_translation(rect, &matrix_input[2], selected, object_matrix, (Vector3){matrix.m12, matrix.m13, translate_Z}, objects);


    rect.y += 40;
    float rotate_X = input_value_matrix(selected, &matrix_input[3], mouse_pos, rect, rotation.x * RAD2DEG, "X rotation");
    apply_rotation(rect, &matrix_input[3], selected, object_matrix, (Vector3){rotate_X * DEG2RAD, rotation.y, rotation.z}, objects);
    
    rect.y += 30;
    float rotate_Y = input_value_matrix(selected, &matrix_input[4], mouse_pos, rect, rotation.y * RAD2DEG, "Y rotation");
    apply_rotation(rect, &matrix_input[4], selected, object_matrix, (Vector3){rotation.x, rotate_Y * DEG2RAD, rotation.z}, objects);
    
    rect.y += 30;
    float rotate_Z = input_value_matrix(selected, &matrix_input[5], mouse_pos, rect, rotation.z * RAD2DEG, "Z rotation");
    apply_rotation(rect, &matrix_input[5], selected, object_matrix, (Vector3){rotation.x, rotation.y, rotate_Z * DEG2RAD}, objects);

    
    rect.y += 40;
    float scale_X = input_value_matrix(selected, &matrix_input[6], mouse_pos, rect, scale.x, "X scale");
    apply_scale(rect, &matrix_input[6], selected, object_matrix, (Vector3){scale_X, scale.y, scale.z}, objects);

    rect.y += 30;
    float scale_Y = input_value_matrix(selected, &matrix_input[7], mouse_pos, rect, scale.y, "Y scale");
    apply_scale(rect, &matrix_input[7], selected, object_matrix, (Vector3){scale.x, scale_Y, scale.z}, objects);

    rect.y += 30;
    float scale_Z = input_value_matrix(selected, &matrix_input[8], mouse_pos, rect, scale.z, "Z scale");
    apply_scale(rect, &matrix_input[8], selected, object_matrix, (Vector3){scale.x, scale.y, scale_Z}, objects);
}