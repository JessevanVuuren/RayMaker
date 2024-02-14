#include "components.h"

void draw_graph(int amount_of_lines, int size_between_lines) {
    for (int i = 0; i < amount_of_lines; i++) {
        float step = size_between_lines - (2 * amount_of_lines * i) / (float)(amount_of_lines - 1);
        float point_one = -sqrtf(powf(amount_of_lines, 2) - powf(step, 2));
        float point_two = sqrtf(powf(amount_of_lines, 2) - powf(step, 2));

        DrawLine3D((Vector3){point_one, 0, step}, (Vector3){point_two, 0, step}, GetColor(0x454545FF));
        DrawLine3D((Vector3){step, 0, point_one}, (Vector3){step, 0, point_two}, GetColor(0x454545FF));
    }
}


XYZcontrol init_XYZ_controls() {
    XYZcontrol xyz;
    xyz.hidden_box = GenMeshCube(20000, 20000, 20000);

    xyz.x.ray.hit = false;
    xyz.y.ray.hit = false;
    xyz.z.ray.hit = false;

    xyz.x.axis = (Vector3){1, 0, 0};
    xyz.y.axis = (Vector3){0, 1, 0};
    xyz.z.axis = (Vector3){0, 0, 1};

    xyz.x.rotation_axis = (Vector3){0, 1, 0};
    xyz.y.rotation_axis = (Vector3){0, 0, 1};
    xyz.z.rotation_axis = (Vector3){1, 0, 0};

    xyz.x.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.y.hit_box.mesh = GenMeshPlane(7.5, .6, 1, 1);
    xyz.z.hit_box.mesh = GenMeshPlane(.6, 7.5, 1, 1);

    xyz.x.rotation_box.material = LoadMaterialDefault();
    xyz.x.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x00FF00FF);

    xyz.y.rotation_box.material = LoadMaterialDefault();
    xyz.y.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0x0000FFFF);

    xyz.z.rotation_box.material = LoadMaterialDefault();
    xyz.z.rotation_box.material.maps[MATERIAL_MAP_DIFFUSE].color = GetColor(0xFF0000FF);

    xyz.x.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.y.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);
    xyz.z.rotation_box.mesh = GenMeshRing(.1, 7.4, 7.8, 60);

    return xyz;
}

void draw_xyz_control(Vector3 target, EditMode mode, Camera3D cam, XYZcontrol *xyz) {
    Vector3 end_pos_x_axis = Vector3Add(target, (Vector3){7.5f, 0, 0});
    Vector3 end_pos_y_axis = Vector3Add(target, (Vector3){0, 7.5f, 0});
    Vector3 end_pos_z_axis = Vector3Add(target, (Vector3){0, 0, -7.5f});

    if (mode == MOVE) {
        DrawCylinderEx(end_pos_x_axis, (Vector3){end_pos_x_axis.x + 1, end_pos_x_axis.y, end_pos_x_axis.z}, .25, 0, 30, GetColor(0xFF0000FF));
        DrawCylinderEx(end_pos_y_axis, (Vector3){end_pos_y_axis.x, end_pos_y_axis.y + 1, end_pos_y_axis.z}, .25, 0, 30, GetColor(0x00FF00FF));
        DrawCylinderEx(end_pos_z_axis, (Vector3){end_pos_z_axis.x, end_pos_z_axis.y, end_pos_z_axis.z - 1}, .25, 0, 30, GetColor(0x0000FFFF));
    };
    if (mode == SCALE) {
        DrawCube(end_pos_x_axis, .65, .65, .65, GetColor(0xFF0000FF));
        DrawCube(end_pos_y_axis, .65, .65, .65, GetColor(0x00FF00FF));
        DrawCube(end_pos_z_axis, .65, .65, .65, GetColor(0x0000FFFF));
    };
    if (mode == ROTATE) {
        DrawMesh(xyz->x.rotation_box.mesh, xyz->x.rotation_box.material, xyz->x.rotation_box.matrix);
        DrawMesh(xyz->y.rotation_box.mesh, xyz->y.rotation_box.material, xyz->y.rotation_box.matrix);
        DrawMesh(xyz->z.rotation_box.mesh, xyz->z.rotation_box.material, xyz->z.rotation_box.matrix);
    };

    DrawLine3D(target, end_pos_x_axis, GetColor(0xFF0000FF));
    DrawLine3D(target, end_pos_y_axis, GetColor(0x00FF00FF));
    DrawLine3D(target, end_pos_z_axis, GetColor(0x0000FFFF));


    float angle_x = atan2f(cam.position.z, cam.position.y);
    float angle_z = atan2f(cam.position.x, cam.position.y);
    float angle_y = atan2f(cam.position.z, cam.position.x);

    end_pos_x_axis.x -= 7.5f * .5f;
    end_pos_y_axis.y -= 7.5f * .5f;
    end_pos_z_axis.z += 7.5f * .5f;


    xyz->x.rotation_box.matrix = Vector3Translate(target);
    xyz->y.rotation_box.matrix = MatrixMultiply(MatrixRotateX(90 * DEG2RAD), Vector3Translate(target));
    xyz->z.rotation_box.matrix = MatrixMultiply(MatrixRotateZ(90 * DEG2RAD), Vector3Translate(target));

    xyz->x.hit_box.matrix = MatrixMultiply(MatrixRotateX(angle_x), Vector3Translate(end_pos_x_axis));
    xyz->y.hit_box.matrix = MatrixMultiply(MatrixRotateY(-angle_y), Vector3Translate(end_pos_y_axis));
    xyz->z.hit_box.matrix = MatrixMultiply(MatrixRotateZ(-angle_z), Vector3Translate(end_pos_z_axis));
    xyz->y.hit_box.matrix = MatrixMultiply(MatrixRotateZ(-(90 * DEG2RAD)), xyz->y.hit_box.matrix);
}

Matrix move_object(Camera cam, Selected *selected, Mesh cube, Vector2 camera_pos, AxisControl xyz, EditMode mode, Object *object) {
    Selected selected_object = *selected;
    Ray ray = GetMouseRay(GetMousePosition(), cam);

    float angle = atan2f(camera_pos.x, camera_pos.y);
    float scale = 10000 / sqrtf(powf(camera_pos.x, 2) + powf(camera_pos.y, 2));

    Vector3 inverse_axis = Vector3Scale(Vector3AddValue(xyz.axis, -1), -1);
    Vector3 opposite_cam = Vector3Scale(cam.position, -scale);
    Vector3 current_axis = Vector3Add(Vector3Multiply(opposite_cam, inverse_axis), selected_object.pos);


    Matrix translation = MatrixTranslate(current_axis.x, current_axis.y, current_axis.z);
    Matrix rotation = MatrixRotate(xyz.axis, angle);
    Matrix matrix = MatrixMultiply(rotation, translation);

    if (mode == ROTATE) {
        Vector3 hidden_rotation_cube = Vector3Multiply((Vector3){-10000, -10000, 10000}, xyz.rotation_axis);
        Vector3 ww = Vector3Add(selected_object.pos, hidden_rotation_cube);
        matrix = Vector3Translate(ww);
    }

    // Material mat = LoadMaterialDefault();
    // mat.maps[MATERIAL_MAP_DIFFUSE].color = PURPLE;
    // DrawMesh(cube, mat, matrix);

    Vector3 hit_point = GetRayCollisionMesh(ray, cube, matrix).point;
    Vector3 hit_point_offset = Vector3Subtract(hit_point, Vector3Multiply(xyz.axis, xyz.ray.point));
    Vector3 offset_current_axis = Vector3Multiply(xyz.axis, hit_point_offset);

    Matrix manipulated_matrix;
    if (mode == MOVE) {
        manipulated_matrix = MatrixMultiply(selected_object.object.model.transform, Vector3Translate(offset_current_axis));
    }
    if (mode == ROTATE) {
        Vector3 base_point = Vector3Subtract(xyz.ray.point, selected_object.pos);
        Vector3 drag_point = Vector3Subtract(hit_point, selected_object.pos);

        float rotate_angle = Vector3Angle(base_point, drag_point);
        Vector3 cross_product = Vector3CrossProduct(base_point, drag_point);

        if (getAxisValue(xyz.rotation_axis, cross_product) < 0) rotate_angle *= -1;

        Matrix rotate = MatrixRotate(xyz.rotation_axis, rotate_angle);
        Matrix rotate_at_origin = MatrixMultiply(MatrixIdentity(), rotate);
        Matrix rotate_to_position = MatrixMultiply(selected_object.object.model.transform, rotate_at_origin);

        manipulated_matrix = selected_object.object.model.transform;
        CopyRotationMatrix(&rotate_to_position, &manipulated_matrix);
    }

    if (mode == SCALE) {
        offset_current_axis.z = -offset_current_axis.z;
        manipulated_matrix = selected_object.object.model.transform;
        Vector3 add_base_one = Vector3AddValue(offset_current_axis, 1);
        Matrix matrix_scaled_up = MatrixScale(add_base_one.x, add_base_one.y, add_base_one.z);
        Matrix matrix_with_rotation = MatrixMultiply(selected_object.object.model.transform, matrix_scaled_up);
        CopyRotationMatrix(&matrix_with_rotation, &manipulated_matrix);
    }
    return manipulated_matrix;
}

void update_position() {
}

Matrix set_matrix_scale(Matrix matrix, Vector3 scale) {
    Vector3 X_norm = Vector3Normalize((Vector3){matrix.m0, matrix.m1, matrix.m2});
    Vector3 Y_norm = Vector3Normalize((Vector3){matrix.m4, matrix.m5, matrix.m6});
    Vector3 Z_norm = Vector3Normalize((Vector3){matrix.m8, matrix.m9, matrix.m10});

    matrix.m0 = X_norm.x * scale.x;
    matrix.m1 = X_norm.y * scale.x;
    matrix.m2 = X_norm.z * scale.x;

    matrix.m4 = Y_norm.x * scale.y;
    matrix.m5 = Y_norm.y * scale.y;
    matrix.m6 = Y_norm.z * scale.y;

    matrix.m8 = Z_norm.x * scale.z;
    matrix.m9 = Z_norm.y * scale.z;
    matrix.m10 = Z_norm.z * scale.z;
    return matrix;
}

Matrix set_matrix_rotation(Matrix matrix, Vector3 axis_angle) {
    Vector3 scale = GetScaleFromMatrix(matrix);
    Vector3 translation = (Vector3){matrix.m12, matrix.m13, matrix.m14};

    Quaternion rotation = QuaternionFromEuler(axis_angle.x, axis_angle.y, axis_angle.z);
    Matrix rotation_matrix = QuaternionToMatrix(rotation);
    return MatrixMultiply(rotation_matrix, Vector3Translate(translation));
}