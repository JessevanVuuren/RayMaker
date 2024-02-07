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

void component_list(Object *objects, int size, int width, int height) {
    int border_size = 10;
    int list_height = 350;
    int list_width = 300;
    int item_distance = 5;
    DrawRectangle(width - list_width + border_size, border_size, list_width - border_size * 2, list_height - border_size * 2, GetColor(0x181818FF));

    for (int i = 0; i < size; i++)
    {
        Object o = objects[i];
        DrawText(o.name, width - list_width + border_size + item_distance, border_size + (i + 1) * item_distance, 15, WHITE);
    }
    

}