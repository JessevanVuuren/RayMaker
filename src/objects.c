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
        Material mat = LoadMaterialDefault();
        mat.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFFDD3355);
        DrawMesh(o.model.meshes[0], mat, o.model.transform);
    }
}

Object load_object(char *model, char *texture, int id, char *name) {
    Object object;
    object.id = id;
    object.name = name;
    object.model = LoadModel(model);
    object.texture = LoadTexture(texture);
    object.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = object.texture;
    return object;
}