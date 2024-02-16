#include "components.h"

Selected update_selected(Object object, int index, bool is_selected) {
    Selected selected;
    selected.object = object;
    selected.pos = getMatrixPosition(object.model.transform);
    selected.index = index;
    selected.is_selected = is_selected;
    return selected;
}

void give_selected_color(Object o) {
    Material mat = LoadMaterialDefault();
    mat.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFFDD3355);
    DrawMesh(o.model.meshes[0], mat, o.model.transform);
}

void draw_models(Object *objects, Selected selected) {
    bool collection_selected = false;
    for (int i = 0; i < arrlen(objects); i++) {
        Object o = objects[i];

        if (o.is_single_object) {
            DrawModel(o.model, Vector3Zero(), 1, WHITE);

            if (selected.object.id == o.id && selected.is_selected) {
                give_selected_color(o);
            }
        } else {
            collection_selected = true;
        }
    }

    for (int i = 0; i < arrlen(objects); i++) {
        Object col_o = objects[i];
        if (col_o.collection_id == selected.object.id && col_o.is_single_object && selected.is_selected && col_o.is_collection == true) {
            give_selected_color(col_o);
        }
    }
}

Object load_object_glb(char *model, int id, char *name) {
    Object object;
    object.id = id;
    object.name = name;
    object.model = LoadModel(model);

    return object;
}

void load_object(Object **objects, char *model, char *texture, char *name) {
    int start_id = arrlen(*objects);
    Model loaded_model = LoadObj(model);
    loaded_model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(texture);

    if (loaded_model.meshCount > 1) {
        Object object1;
        object1.id = start_id;
        object1.collection_id = start_id;
        object1.name = name;
        object1.is_single_object = false;
        arrput(*objects, object1);

        for (int i = 0; i < loaded_model.meshCount; i++) {

            Object o = {
                .is_collection = true,
                .collection_id = start_id,
                .is_single_object = true,
                .model = LoadModelFromMesh(loaded_model.meshes[i]),
                .name = loaded_model.names[i],
                o.id = start_id + i + 1,
            };
            o.model.materials = loaded_model.materials;
            arrput(*objects, o);
        }
    } else {
        Object object;
        object.id = start_id;
        object.collection_id = start_id;
        object.is_single_object = true;
        object.is_collection = false;
        object.name = name;
        object.model = loaded_model;

        arrput(*objects, object);
    }
}