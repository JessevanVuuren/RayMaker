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
        for (int i = 0; i < o.model.meshCount; i++) {
            DrawMesh(o.model.meshes[i], mat, o.model.transform);
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

void load_object(Object **objects, char *model, char *texture) {
    int start_id = arrlen(*objects);
    char *name = (char*)GetFileNameWithoutExt(model);
    Object object;
    object.id = start_id;
    object.collection = (Collection){.id = -1, .is_part_of = false, .name = "none"};
    object.name = name;
    object.model = LoadObj(model);
    object.model.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = LoadTexture(texture);

    if (object.model.meshCount > 1) {
        for (int i = 0; i < object.model.meshCount; i++) {
            Collection collection;
            collection.id = start_id;
            collection.is_part_of = true;
            collection.name = malloc(strlen(name) + 1);
            strcpy(collection.name, name);

            Object o = {
                .collection = collection,
                .model = LoadModelFromMesh(object.model.meshes[i]),
                .name = object.model.names[i],
                o.id = start_id + i,
            };
            o.model.materials = object.model.materials;
            arrput(*objects, o);
        }
    } else {
        arrput(*objects, object);
    }
}