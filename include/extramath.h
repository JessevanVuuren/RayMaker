#include "stdio.h"

Vector3 Vector3MultiplyValue(Vector3 v, float multiply)
{
    Vector3 result = { v.x * multiply, v.y * multiply, v.z * multiply };

    return result;
}

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

Vector3 GetRotationFromMatrix(Matrix mat) {
    Vector3 rotation;

    // Assuming the angles are in radians
    // Pitch (X-axis rotation)
    float sinPitch = -mat.m2;
    float cosPitch = sqrt(1.0f - sinPitch * sinPitch);
    rotation.x = atan2(sinPitch, cosPitch);

    // Yaw (Y-axis rotation)
    float sinYaw = mat.m8;
    float cosYaw = mat.m10;
    rotation.y = atan2(sinYaw, cosYaw);

    // Roll (Z-axis rotation)
    float sinRoll = mat.m1;
    float cosRoll = mat.m0;
    rotation.z = atan2(sinRoll, cosRoll);

    return rotation;
}