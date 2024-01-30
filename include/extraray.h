#include "stdio.h"


Matrix Vector3Translate(Vector3 pos) {
    return MatrixTranslate(pos.x, pos.y, pos.z);
}

Vector3 getMatrixPosition(Matrix mat) {
    Vector3 translation;
    translation.x = mat.m12;
    translation.y = mat.m13;
    translation.z = mat.m14;
    return translation;
}

Vector3 makeVector3(float value) {
    return (Vector3) {value, value, value};
}

void printV(Vector3 vec) {
    printf("vec: X: %f, Y: %f, Z: %f\n", vec.x, vec.y, vec.z);
}

float getAxisValue(Vector3 axis, Vector3 vec) {
    float value = 0;
    if (axis.x == 1) value += vec.x;
    if (axis.y == 1) value += vec.y;
    if (axis.z == 1) value += vec.z;
    return value;
}

float ExtractRotationAngleFromMatrix(Matrix mat) {
    float trace = mat.m0 + mat.m5 + mat.m10; // Sum of the diagonal elements
    float angle = acosf((trace - 1.0f) / 2.0f); // angle in radians

    return angle;
}

Mesh GenMeshRing(float height, float inner_radius, float outer_radius, int slices) {
    Mesh mesh = {0};

    mesh.vertexCount = slices * 4;
    mesh.triangleCount = slices * 8;

    mesh.vertices = (float *)malloc(mesh.vertexCount * 3 * sizeof(float));
    mesh.indices = (unsigned short *)malloc(mesh.triangleCount * 3 * sizeof(unsigned short));

    for (int i = 0; i < slices; i++) {
        float x = cosf(2 * PI * ((float)i / (slices)));
        float z = sinf(2 * PI * ((float)i / (slices)));

        mesh.vertices[i * 12 + 0] = x * outer_radius;
        mesh.vertices[i * 12 + 1] = 0;
        mesh.vertices[i * 12 + 2] = z * outer_radius;

        mesh.vertices[i * 12 + 3] = x * outer_radius;
        mesh.vertices[i * 12 + 4] = height;
        mesh.vertices[i * 12 + 5] = z * outer_radius;

        mesh.vertices[i * 12 + 6] = x * inner_radius;
        mesh.vertices[i * 12 + 7] = 0;
        mesh.vertices[i * 12 + 8] = z * inner_radius;

        mesh.vertices[i * 12 + 9] = x * inner_radius;
        mesh.vertices[i * 12 + 10] = height;
        mesh.vertices[i * 12 + 11] = z * inner_radius;
    }


    int mod = slices * 4;
    for (int i = 0; i < slices; i++) {
        int step_size = 4 * i;

        mesh.indices[i * 24 + 0] = (0 + step_size) % mod;
        mesh.indices[i * 24 + 1] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 2] = (4 + step_size) % mod;

        mesh.indices[i * 24 + 3] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 4] = (5 + step_size) % mod;
        mesh.indices[i * 24 + 5] = (4 + step_size) % mod;

        mesh.indices[i * 24 + 6] = (6 + step_size) % mod;
        mesh.indices[i * 24 + 7] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 8] = (2 + step_size) % mod;

        mesh.indices[i * 24 + 9] = (7 + step_size) % mod;
        mesh.indices[i * 24 + 10] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 11] = (6 + step_size) % mod;

        mesh.indices[i * 24 + 12] = (1 + step_size) % mod;
        mesh.indices[i * 24 + 13] = (3 + step_size) % mod;
        mesh.indices[i * 24 + 14] = (7 + step_size) % mod;

        mesh.indices[i * 24 + 15] = (7 + step_size) % mod;
        mesh.indices[i * 24 + 16] = (5 + step_size) % mod;
        mesh.indices[i * 24 + 17] = (1 + step_size) % mod;

        mesh.indices[i * 24 + 18] = (6 + step_size) % mod;
        mesh.indices[i * 24 + 19] = (2 + step_size) % mod;
        mesh.indices[i * 24 + 20] = (0 + step_size) % mod;

        mesh.indices[i * 24 + 21] = (0 + step_size) % mod;
        mesh.indices[i * 24 + 22] = (4 + step_size) % mod;
        mesh.indices[i * 24 + 23] = (6 + step_size) % mod;
    }

    UploadMesh(&mesh, false);

    return mesh;
}

Matrix RotationMatrixFromEuler(float x, float y, float z) {
    Matrix rx = MatrixRotateX(x);
    Matrix ry = MatrixRotateY(y);
    Matrix rz = MatrixRotateZ(z);

    // Combine rotations: first Z, then Y, then X
    return MatrixMultiply(MatrixMultiply(rz, ry), rx);
}

void MatrixToAxisAngle(Matrix mat, Vector3* axis, float* angle) {
    float trace = mat.m0 + mat.m5 + mat.m10;
    *angle = acosf((trace - 1.0f) / 2.0f);

    axis->x = mat.m6 - mat.m9;
    axis->y = mat.m8 - mat.m2;
    axis->z = mat.m1 - mat.m4;

    float norm = sqrtf(axis->x * axis->x + axis->y * axis->y + axis->z * axis->z);
    if (norm != 0.0f) { // to avoid division by zero
        axis->x /= norm;
        axis->y /= norm;
        axis->z /= norm;
    }
}