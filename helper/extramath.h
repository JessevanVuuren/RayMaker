#include "raylib.h"
#include "raymath.h"

Vector3 Vector3MultiplyValue(Vector3 v, float multiply)
{
    Vector3 result = { v.x * multiply, v.y * multiply, v.z * multiply };

    return result;
}

Vector3 getMatrixTranslation(Matrix mat) {
    Vector3 translation;
    translation.x = mat.m12;
    translation.y = mat.m13;
    translation.z = mat.m14;
    return translation;
}

void printV(Vector3 vec) {
    printf("vec: X: %f, Y: %f, X: %f\n", vec.x, vec.y, vec.z);
}
