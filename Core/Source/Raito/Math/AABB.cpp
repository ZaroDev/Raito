#include "pch.h"
#include "AABB.h"

namespace Raito::Math
{
    AABB::AABB()
    {
        SetNull();
    }

    AABB::AABB(const V3& center, glm::float32_t radius)
    {
        SetNull();
        Extend(center, radius);
    }

    AABB::AABB(const V3& p1, const V3& p2)
    {
        SetNull();
        Extend(p1);
        Extend(p2);
    }

    AABB::AABB(const AABB& aabb)
    {
        SetNull();
        Extend(aabb);
    }


    void AABB::Extend(glm::float32_t val)
    {
        if (!IsNull())
        {
            m_Min -= V3(val);
            m_Max += V3(val);
        }
    }

    void AABB::Extend(const V3& p)
    {
        if (!IsNull())
        {
            m_Min = glm::min(p, m_Min);
            m_Max = glm::max(p, m_Max);
        }
        else
        {
            m_Min = p;
            m_Max = p;
        }
    }

    void AABB::Extend(const V3& p, glm::float32_t radius)
    {
        V3 r(radius);
        if (!IsNull())
        {
            m_Min = glm::min(p - r, m_Min);
            m_Max = glm::max(p + r, m_Max);
        }
        else
        {
            m_Min = p - r;
            m_Max = p + r;
        }
    }

    void AABB::Extend(const AABB& aabb)
    {
        if (!aabb.IsNull())
        {
            Extend(aabb.m_Min);
            Extend(aabb.m_Max);
        }
    }

    void AABB::Extend(const V3* pointsArray, int numCount)
    {
        if (!pointsArray)
            return;
        for (int i = 0; i < numCount; ++i)
            Extend(pointsArray[i]);
    }

    void AABB::ExtendDisk(const V3& c, const V3& n, glm::float32_t r)
    {
        if (glm::length(n) < 1.e-12) { Extend(c); return; }
        V3 norm = glm::normalize(n);
        glm::float32_t x = sqrt(1 - norm.x) * r;
        glm::float32_t y = sqrt(1 - norm.y) * r;
        glm::float32_t z = sqrt(1 - norm.z) * r;
        Extend(c + V3(x, y, z));
        Extend(c - V3(x, y, z));
    }

    V3 AABB::GetDiagonal() const
    {
        if (!IsNull())
            return m_Max - m_Min;
        else
            return V3(0);
    }

    glm::float32_t AABB::GetLongestEdge() const
    {
        return glm::compMax(GetDiagonal());
    }

    glm::float32_t AABB::GetShortestEdge() const
    {
        return glm::compMin(GetDiagonal());
    }

    V3 AABB::GetCenter() const
    {
        if (!IsNull())
        {
            V3 d = GetDiagonal();
            return m_Min + (d * glm::float32_t(0.5));
        }
        else
        {
            return V3(0.0);
        }
    }

    void AABB::Translate(const V3& v)
    {
        if (!IsNull())
        {
            m_Min += v;
            m_Max += v;
        }
    }

    void AABB::Scale(const V3& s, const V3& o)
    {
        if (!IsNull())
        {
            m_Min -= o;
            m_Max -= o;

            m_Min *= s;
            m_Max *= s;

            m_Min += o;
            m_Max += o;
        }
    }

    bool AABB::Overlaps(const AABB& bb) const
    {
        if (IsNull() || bb.IsNull())
            return false;

        if (bb.m_Min.x > m_Max.x || bb.m_Max.x < m_Min.x)
            return false;
        else if (bb.m_Min.y > m_Max.y || bb.m_Max.y < m_Min.y)
            return false;
        else if (bb.m_Min.z > m_Max.z || bb.m_Max.z < m_Min.z)
            return false;

        return true;
    }

    AABB::IntersectionType AABB::Intersect(const AABB& b) const
    {
        if (IsNull() || b.IsNull())
            return OUTSIDE;

        if ((m_Max.x < b.m_Min.x) || (m_Min.x > b.m_Max.x) ||
            (m_Max.y < b.m_Min.y) || (m_Min.y > b.m_Max.y) ||
            (m_Max.z < b.m_Min.z) || (m_Min.z > b.m_Max.z))
        {
            return OUTSIDE;
        }

        if ((m_Min.x <= b.m_Min.x) && (m_Max.x >= b.m_Max.x) &&
            (m_Min.y <= b.m_Min.y) && (m_Max.y >= b.m_Max.y) &&
            (m_Min.z <= b.m_Min.z) && (m_Max.z >= b.m_Max.z))
        {
            return INSIDE;
        }

        return INTERSECT;
    }


    bool AABB::IsSimilarTo(const AABB& b, glm::float32_t diff) const
    {
        if (IsNull() || b.IsNull()) return false;

        V3 acceptable_diff = ((GetDiagonal() + b.GetDiagonal()) / glm::float32_t(2.0)) * diff;
        V3 min_diff(m_Min - b.m_Min);
        min_diff = V3(fabs(min_diff.x), fabs(min_diff.y), fabs(min_diff.z));
        if (min_diff.x > acceptable_diff.x) return false;
        if (min_diff.y > acceptable_diff.y) return false;
        if (min_diff.z > acceptable_diff.z) return false;
        V3 max_diff(m_Max - b.m_Max);
        max_diff = V3(fabs(max_diff.x), fabs(max_diff.y), fabs(max_diff.z));
        if (max_diff.x > acceptable_diff.x) return false;
        if (max_diff.y > acceptable_diff.y) return false;
        if (max_diff.z > acceptable_diff.z) return false;
        return true;
    }

    V3 AABB::Size() const
    {
        return m_Max - m_Min;
    }

    V3 AABB::HalfSize() const
    {
        return Size() / 2.f;
    }
}