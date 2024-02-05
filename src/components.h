#ifndef COMPONENTS_H
#define COMPONENTS_H

#include "raylib.h"


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

Button load_button(char *img_path, char *name, int x, int y);
void render_buttons(Button *buttons, int size);
XYZcontrol init_XYZ_controls(void);

#endif 




