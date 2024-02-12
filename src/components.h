#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"
#include "extraray.h"
#include "raygui.h"

#include "stb_ds.h"

typedef enum { MOVE,
               SCALE,
               ROTATE,
               NONE

} EditMode;


typedef struct {
    int id;
    Mesh mesh;
    Matrix matrix;
    Material material;
} HitObject;


typedef struct {
    Vector3 axis;
    RayCollision ray;
    HitObject hit_box;
    Vector3 rotation_axis;
    HitObject rotation_box;
} AxisControl;

typedef struct {
    AxisControl x;
    AxisControl y;
    AxisControl z;
    Mesh hidden_box;
} XYZcontrol;

typedef struct {
    char *name;
    Image img;
    Vector2 pos;
    bool pressed;
    Rectangle rec;
    char *img_path;
    Texture2D texture;
} Button;

typedef struct {
    int id;
    char *name;
    bool is_part_of;
} Collection;

typedef struct {
    char *name;
    int id;
    Model model;
    Collection collection;
} Object;



typedef struct {
    int index;
    Vector3 pos;
    Object object;
    bool is_selected;
} Selected;


// interface.c
Button load_button(char *img_path, char *name, int x, int y);
void render_buttons(Button *buttons, int size);
void button_pressed(int button_index, EditMode *edit);
void component_list(Object *objects, Selected selected, int width, int height, Font font);
bool mouse_is_in_ui_elements(Rectangle box[], int size);
Rectangle *update_ui_box(int new_width, int new_height, int size);
void matrix_display();

// xyz_control.c
XYZcontrol init_XYZ_controls(void);
void draw_xyz_control(Vector3 target, EditMode mode, Camera3D cam, XYZcontrol *xyz);
Matrix move_object(Camera cam, Selected *selected, Mesh cube, Vector2 camera_pos, AxisControl xyz, EditMode mode, Object *object);
void draw_graph(int amount_of_lines, int size_between_lines);

// objects.c
Selected update_selected(Object object, int index, bool is_selected, Selected curr_selected);
void draw_model(Object o, Selected selected);
void load_object(Object **objects, char *model, char *texture);
Model LoadObj(const char *filename);

#endif 




