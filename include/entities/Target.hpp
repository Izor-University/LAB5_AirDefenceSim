#pragma once
#include "../physics/IPhysicalObject.hpp"
#include <string>

class Target : public IPhysicalObject {
private:
    std::string Name;
    Vector3D Position;
    Vector3D Velocity;
    double Radius;
    double Rcs;
    Material TargetMaterial;

public:
    Target(std::string Name, Vector3D Position, Vector3D Velocity, double Radius, double Rcs, Material Mat = Material(0.8, 0.0));

    std::string GetName() const override;
    Vector3D GetVelocity() const override;
    Material GetMaterial() const override;
    double GetRcs() const override;
    Vector3D GetPosition() const;

    void UpdatePhysics(double DeltaTime) override;
    std::optional<HitRecord> CalculateIntersection(const ComplexWave& Wave) const override;
};