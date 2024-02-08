#include "pch.h"
#include "Math.h"

#include <glm/gtc/matrix_transform.hpp>

namespace Raito::Math
{
    Mat4 CreateTransform(const V3& translation, const Quaternion& rotation, const V3& scale)
    {
        return glm::translate(Mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(Mat4(1.0f), scale);
    }
}
