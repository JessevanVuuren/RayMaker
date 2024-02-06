#include "components.h"

Selected update_selected(Object object, int index, bool is_selected, Selected curr_selected) {
    Selected selected;
    selected.object = object;
    selected.pos = getMatrixPosition(object.model.transform);
    selected.index = index;
    selected.is_selected = is_selected;
    return selected;
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