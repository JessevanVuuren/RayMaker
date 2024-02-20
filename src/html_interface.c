#include "components.h"
#include <emscripten.h>

EMSCRIPTEN_KEEPALIVE
void set_edit_mode(int value) {
    if (value == 0) control_mode = MOVE;
    if (value == 1) control_mode = ROTATE;
    if (value == 2) control_mode = SCALE;
}

void callJSFunction(float value) {
    EM_ASM_ARGS({testing($0)}, value);
}

EMSCRIPTEN_KEEPALIVE
void string_test(char *name) {
    printf("name: %s\n", name);
}

EMSCRIPTEN_KEEPALIVE
void set_upload_model(char *model_path, char *texture_path, char *name) {
    load_object(model_path, texture_path, name);
}

EMSCRIPTEN_KEEPALIVE
char *get_object_list() {
    char *list_data = malloc((arrlen(objects) * 200));

    int pos = 0;
    for (int i = 0; i < arrlen(objects); i++)
    {
        Object o = objects[i];
           
        pos += sprintf(&list_data[pos], "%s-%d-%d-%d-%d|", o.name, o.id, o.collection_id, o.is_collection, o.is_part_of);
    }
    
    return list_data;
}