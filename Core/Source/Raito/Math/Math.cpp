#include "pch.h"
#include "Math.h"


namespace Raito::Math
{
    Matrix CreateTransform(const v3& translation, const Quaternion& rotation, const v3& scale)
    {
        Vector quaternion = DirectX::XMLoadFloat4(&rotation);

        Matrix rotationMatrix = DirectX::XMMatrixRotationQuaternion(quaternion);
        Matrix translationMatrix = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
        Matrix scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

        return translationMatrix * rotationMatrix * scaleMatrix;
    }
}
