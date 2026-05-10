#pragma once
#include "../core/Vector3D.hpp"
#include "../core/ComplexWave.hpp"
#include "../core/Material.hpp"
#include "../core/IntersectionMath.hpp"
#include <optional>
#include <string>

class IPhysicalObject {
public:
    virtual ~IPhysicalObject() = default;
    virtual std::string GetName() const = 0;
    virtual std::optional<HitRecord> CalculateIntersection(const ComplexWave& Wave) const = 0;
    virtual Material GetMaterial() const = 0;
    virtual double GetRcs() const = 0;
    virtual Vector3D GetPosition() const = 0;
    virtual Vector3D GetVelocity() const = 0;
    virtual void UpdatePhysics(double DeltaTime) = 0;
};