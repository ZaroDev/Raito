#pragma once

#include "Math/MathTypes.h"

namespace Raito::Math
{
	matrix CreateTransform(const v3& translation, const v4& rotation, const v3& scale)
    {
        vector quaternion = DirectX::XMLoadFloat4(&rotation);

       matrix rotationMatrix = DirectX::XMMatrixRotationQuaternion(quaternion);
       matrix translationMatrix = DirectX::XMMatrixTranslation(translation.x, translation.y, translation.z);
       matrix scaleMatrix = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

       return translationMatrix * rotationMatrix * scaleMatrix;
	}
}