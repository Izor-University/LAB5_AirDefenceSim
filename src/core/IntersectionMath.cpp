#include "../../include/core/IntersectionMath.hpp"

namespace IntersectionMath {

    std::optional<HitRecord> RayIntersectSphere(const Vector3D& RayOrigin, const Vector3D& RayDirection, 
                                                const Vector3D& SphereCenter, double SphereRadius) {
        Vector3D L = SphereCenter - RayOrigin;
        double Tca = L.DotProduct(RayDirection);
        if (Tca < 0.0) return std::nullopt;

        double D2 = L.DotProduct(L) - Tca * Tca;
        double Radius2 = SphereRadius * SphereRadius;
        if (D2 > Radius2) return std::nullopt;

        double Thc = std::sqrt(Radius2 - D2);
        double T0 = Tca - Thc;
        double T1 = Tca + Thc;

        double HitDistance = -1.0;
        if (T0 > 0.0) HitDistance = T0;
        else if (T1 > 0.0) HitDistance = T1;

        if (HitDistance < 0.0) return std::nullopt;

        HitRecord Record;
        Record.Distance = HitDistance;
        Record.HitPoint = RayOrigin + (RayDirection * HitDistance);
        Record.Normal = (Record.HitPoint - SphereCenter).Normalize(); 
        return Record;
    }

    std::optional<HitRecord> RayIntersectPlane(const Vector3D& RayOrigin, const Vector3D& RayDirection, 
                                               const Vector3D& PlanePoint, const Vector3D& PlaneNormal) {
        double Denominator = PlaneNormal.DotProduct(RayDirection);
        if (std::abs(Denominator) < 1e-6) return std::nullopt;

        Vector3D P0L0 = PlanePoint - RayOrigin;
        double T = P0L0.DotProduct(PlaneNormal) / Denominator;

        if (T >= 0.0) {
            HitRecord Record;
            Record.Distance = T;
            Record.HitPoint = RayOrigin + (RayDirection * T);
            Record.Normal = (Denominator < 0) ? PlaneNormal : (PlaneNormal * -1.0);
            return Record;
        }
        return std::nullopt;
    }
}