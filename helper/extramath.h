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

Matrix Vector2ToMatrix(Vector3 pos) {
    return MatrixTranslate(pos.x, pos.y, pos.z);
}

        // printf("vec: %f, %f, %f\n", ppp.x, ppp.y, ppp.z);
