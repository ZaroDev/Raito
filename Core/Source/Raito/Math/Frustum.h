#pragma once
#include <Raito/Math/MathTypes.h>

namespace Raito::Math
{
	class Frustum
	{
	public:
		Frustum() = default;

		// m = ProjectionMatrix * ViewMatrix 
		Frustum(Mat4 m);

		// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
		NODISCARD bool IsBoxVisible(const V3& minp, const V3& maxp) const;
	private:
		enum Planes
		{
			LEFT = 0,
			RIGHT,
			BOTTOM,
			TOP,
			NEAR_PLANE,
			FAR_PLANE,
			COUNT,
			COMBINATIONS = COUNT * (COUNT - 1) / 2
		};

		template<Planes i, Planes j>
		struct ij2k
		{
			enum { k = i * (9 - i) / 2 + j - 1 };
		};

		template<Planes a, Planes b, Planes c>
		V3 Intersection(const V3* crosses) const;

		V4   m_Planes[COUNT]{};
		V3   m_Points[8]{};
	};
	inline Frustum::Frustum(glm::mat4 m)
	{
		m = glm::transpose(m);
		m_Planes[LEFT] = m[3] + m[0];
		m_Planes[RIGHT] = m[3] - m[0];
		m_Planes[BOTTOM] = m[3] + m[1];
		m_Planes[TOP] = m[3] - m[1];
		m_Planes[NEAR_PLANE] = m[3] + m[2];
		m_Planes[FAR_PLANE] = m[3] - m[2];

		const V3 crosses[COMBINATIONS] = {
			cross(V3(m_Planes[LEFT]),   V3(m_Planes[RIGHT])),
			cross(V3(m_Planes[LEFT]),   V3(m_Planes[BOTTOM])),
			cross(V3(m_Planes[LEFT]),   V3(m_Planes[TOP])),
			cross(V3(m_Planes[LEFT]),   V3(m_Planes[NEAR_PLANE])),
			cross(V3(m_Planes[LEFT]),   V3(m_Planes[FAR_PLANE])),
			cross(V3(m_Planes[RIGHT]),  V3(m_Planes[BOTTOM])),
			cross(V3(m_Planes[RIGHT]),  V3(m_Planes[TOP])),
			cross(V3(m_Planes[RIGHT]),  V3(m_Planes[NEAR_PLANE])),
			cross(V3(m_Planes[RIGHT]),  V3(m_Planes[FAR_PLANE])),
			cross(V3(m_Planes[BOTTOM]), V3(m_Planes[TOP])),
			cross(V3(m_Planes[BOTTOM]), V3(m_Planes[NEAR_PLANE])),
			cross(V3(m_Planes[BOTTOM]), V3(m_Planes[FAR_PLANE])),
			cross(V3(m_Planes[TOP]),    V3(m_Planes[NEAR_PLANE])),
			cross(V3(m_Planes[TOP]),    V3(m_Planes[FAR_PLANE])),
			cross(V3(m_Planes[NEAR_PLANE]),   V3(m_Planes[FAR_PLANE]))
		};

		m_Points[0] = Intersection<LEFT, BOTTOM, NEAR_PLANE>(crosses);
		m_Points[1] = Intersection<LEFT, TOP, NEAR_PLANE>(crosses);
		m_Points[2] = Intersection<RIGHT, BOTTOM, NEAR_PLANE>(crosses);
		m_Points[3] = Intersection<RIGHT, TOP, NEAR_PLANE>(crosses);
		m_Points[4] = Intersection<LEFT, BOTTOM, FAR_PLANE>(crosses);
		m_Points[5] = Intersection<LEFT, TOP, FAR_PLANE>(crosses);
		m_Points[6] = Intersection<RIGHT, BOTTOM, FAR_PLANE>(crosses);
		m_Points[7] = Intersection<RIGHT, TOP, FAR_PLANE>(crosses);

	}

	// http://iquilezles.org/www/articles/frustumcorrect/frustumcorrect.htm
	inline bool Frustum::IsBoxVisible(const V3& minp, const V3& maxp) const
	{
		// check box outside/inside of frustum
		for (int i = 0; i < COUNT; i++)
		{
			if ((dot(m_Planes[i], V4(minp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(maxp.x, minp.y, minp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(minp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(maxp.x, maxp.y, minp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(minp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(maxp.x, minp.y, maxp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(minp.x, maxp.y, maxp.z, 1.0f)) < 0.0) &&
				(dot(m_Planes[i], V4(maxp.x, maxp.y, maxp.z, 1.0f)) < 0.0))
			{
				return false;
			}
		}

		// check frustum outside/inside box
		int out;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].x > maxp.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].x < minp.x) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].y > maxp.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].y < minp.y) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].z > maxp.z) ? 1 : 0); if (out == 8) return false;
		out = 0; for (int i = 0; i < 8; i++) out += ((m_Points[i].z < minp.z) ? 1 : 0); if (out == 8) return false;

		return true;
	}



	template<Frustum::Planes a, Frustum::Planes b, Frustum::Planes c>
	inline V3 Frustum::Intersection(const V3* crosses) const
	{
		const float D = glm::dot(V3(m_Planes[a]), crosses[ij2k<b, c>::k]);
		const V3 res = Mat3(crosses[ij2k<b, c>::k], -crosses[ij2k<a, c>::k], crosses[ij2k<a, b>::k]) *
						V3(m_Planes[a].w, m_Planes[b].w, m_Planes[c].w);
		return res * (-1.0f / D);
	}
}