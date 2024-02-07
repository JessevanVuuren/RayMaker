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

void component_list(Object *objects, Selected selected, int size, int width, int height, Font font) {
    int border_size = 10;
    Vector2 box_size = {300, 350};
    Rectangle box = {width - box_size.x + border_size, border_size, box_size.x - border_size * 2, box_size.y - border_size * 2};
    
    int text_distance = 20;
    DrawRectangle(box.x, box.y, box.width, box.height, GetColor(0x181818FF));
    Vector2 mouse = GetMousePosition();

    for (int i = 0; i < size; i++)
    {
        Object o = objects[i];

        Color background_color = GetColor(0x181818FF);
        if (selected.object.id == o.id && selected.is_selected) {
            background_color = GetColor(0x383838FF);
        }
        
        DrawRectangle(box.x, box.y + i * text_distance, box.width, text_distance, background_color);
        DrawTextEx(font, o.name, (Vector2){box.x + 5, border_size + i * text_distance}, 20, 2, WHITE);
    }
    

}