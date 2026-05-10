#include "../../include/Entities/Target.hpp"

// Реализация конструктора
Target::Target(std::string Name, Vector3D Position, Vector3D Velocity, 
               double Radius, double Rcs, Material Mat)
    : Name(Name), Position(Position), Velocity(Velocity), 
      Radius(Radius), Rcs(Rcs), TargetMaterial(Mat) {}

// Реализация геттеров
std::string Target::GetName() const { return Name; }
Vector3D Target::GetVelocity() const { return Velocity; }
Material Target::GetMaterial() const { return TargetMaterial; }
double Target::GetRcs() const { return Rcs; }
Vector3D Target::GetPosition() const { return Position; }

// Реализация физики
void Target::UpdatePhysics(double DeltaTime) {
    Position = Position + (Velocity * DeltaTime);
}

// Реализация пересечения
std::optional<HitRecord> Target::CalculateIntersection(const ComplexWave& Wave) const {
    return IntersectionMath::RayIntersectSphere(
        Wave.Position, Wave.Direction, Position, Radius
    );
}