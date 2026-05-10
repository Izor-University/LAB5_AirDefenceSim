#pragma once
#include "../physics/IPhysicalObject.hpp"
#include <string>

class Obstacle : public IPhysicalObject {
private:
    std::string Name;
    Vector3D Position;
    Vector3D Normal;
    Material ObstacleMaterial;

public:
    Obstacle(std::string Name, Vector3D Position, Vector3D Normal, Material Mat = Material(0.5, 3.14159));

    std::string GetName() const override;
    Vector3D GetVelocity() const override;
    Material GetMaterial() const override;
    double GetRcs() const override;
    Vector3D GetPosition() const override;

    void UpdatePhysics(double DeltaTime) override;
    std::optional<HitRecord> CalculateIntersection(const ComplexWave& Wave) const override;
};