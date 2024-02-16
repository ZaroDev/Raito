#include "pch.h"
#include "Math.h"

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace Raito::Math
{
	Mat4 CreateTransform(const V3& translation, const Quaternion& rotation, const V3& scale)
	{
		return glm::translate(Mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(Mat4(1.0f), scale);
	}
	void DecomposeTransform(const Mat4& m, V3& translation, Quaternion& rotation, V3& scale)
	{
		V3 view;
		V4 pers;

		glm::decompose(m, scale, rotation, translation, view, pers);

		rotation = glm::conjugate(rotation);
	}
}
