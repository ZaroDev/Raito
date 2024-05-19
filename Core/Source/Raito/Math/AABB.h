#pragma once
#include <Raito/Math/MathTypes.h>

namespace Raito::Math
{
	class AABB
	{
    public:
        /// Builds a null AABB.
        AABB();

        /// Builds an AABB that encompasses a sphere.
        /// \param[in]  center Center of the sphere.
        /// \param[in]  radius Radius of the sphere.
        AABB(const V3& center, glm::float32_t radius);

        /// Builds an AABB that contains the two points.
        AABB(const V3& p1, const V3& p2);

        AABB(const AABB& aabb);
        ~AABB() = default;

        /// Set the AABB as NULL (not set).
        void SetNull() { m_Min = V3(1.0); m_Max = V3(-1.0); }

        /// Returns true if AABB is NULL (not set).
        NODISCARD bool IsNull() const { return m_Min.x > m_Max.x || m_Min.y > m_Max.y || m_Min.z > m_Max.z; }

        /// Extend the bounding box on all sides by \p val.
        void Extend(glm::float32_t val);

        /// Expand the AABB to include point \p p.
        void Extend(const V3& p);

        /// Expand the AABB to include a sphere centered at \p center and of radius \p
        /// radius.
        /// \param[in]  center Center of sphere.
        /// \param[in]  radius Radius of sphere.
        void Extend(const V3& center, glm::float32_t radius);

        /// Expand the AABB to encompass the given \p aabb.
        void Extend(const AABB& aabb);

        void Extend(const V3* pointsArray, int numCount);

        /// Expand the AABB to include a disk centered at \p center, with normal \p
        /// normal, and radius \p radius.
        /// \xxx Untested -- This function is not represented in our unit tests.
        void ExtendDisk(const V3& center, const V3& normal,
            glm::float32_t radius);

        /// Translates AABB by vector \p v.
        void Translate(const V3& v);

        /// Scale the AABB by \p scale, centered around \p origin.
        /// \param[in]  scale  3D vector specifying scale along each axis.
        /// \param[in]  origin Origin of scaling operation. Most useful origin would
        ///                    be the center of the AABB.
        void Scale(const V3& scale, const V3& origin);

        /// Retrieves the center of the AABB.
        NODISCARD V3 GetCenter() const;

        /// Retrieves the diagonal vector (computed as m_Max - m_Min).
        /// If the AABB is NULL, then a vector of all zeros is returned.
        NODISCARD V3 GetDiagonal() const;

        /// Retrieves the longest edge.
        /// If the AABB is NULL, then 0 is returned.
        NODISCARD glm::float32_t GetLongestEdge() const;

        /// Retrieves the shortest edge.
        /// If the AABB is NULL, then 0 is returned.
        NODISCARD glm::float32_t GetShortestEdge() const;

        /// Retrieves the AABB's minimum point.
        NODISCARD V3 GetMin() const { return m_Min; }

        /// Retrieves the AABB's maximum point.
        NODISCARD V3 GetMax() const { return m_Max; }

        /// Returns true if AABBs share a face overlap.
        /// \xxx Untested -- This function is not represented in our unit tests.
        NODISCARD  bool Overlaps(const AABB& bb) const;

        /// Type returned from call to intersect.
        enum IntersectionType { INSIDE, INTERSECT, OUTSIDE };
        /// Returns one of the intersection types. If either of the aabbs are invalid,
        /// then OUTSIDE is returned.
        NODISCARD IntersectionType Intersect(const AABB& bb) const;

        /// Function from SCIRun. Here is a summary of SCIRun's description:
        /// Returns true if the two AABB's are similar. If diff is 1.0, the two
        /// bboxes have to have about 50% overlap each for x,y,z. If diff is 0.0,
        /// they have to have 100% overlap.
        /// If either of the two AABBs is NULL, then false is returned.
        /// \xxx Untested -- This function is not represented in our unit tests.
        NODISCARD bool IsSimilarTo(const AABB& b, glm::float32_t diff = 0.5) const;

        /// Returns the side lengths of this AABB in x, y and z directions.
        /** The returned vector is equal to the diagonal vector of this AABB, i.e. it spans from the
        minimum corner of the AABB to the maximum corner of the AABB.
        @see HalfSize(), Diagonal(). */
        NODISCARD V3 Size() const;

        /// [similarOverload: Size]
        /** Returns Size()/2.
        @see Size(), HalfDiagonal(). */
        NODISCARD V3 HalfSize() const;

    private:

        V3 m_Min;   ///< Minimum point.
        V3 m_Max;   ///< Maximum point.

	};
}