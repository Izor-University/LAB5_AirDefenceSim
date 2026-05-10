#pragma once
#include "Vector3D.hpp"
#include <optional>

struct HitRecord {
    double Distance;
    Vector3D HitPoint;
    Vector3D Normal;
};

namespace IntersectionMath {
    std::optional<HitRecord> RayIntersectSphere(const Vector3D& RayOrigin, const Vector3D& RayDirection,
                                                const Vector3D& SphereCenter, double SphereRadius);

    std::optional<HitRecord> RayIntersectPlane(const Vector3D& RayOrigin, const Vector3D& RayDirection,
                                               const Vector3D& PlanePoint, const Vector3D& PlaneNormal);
}