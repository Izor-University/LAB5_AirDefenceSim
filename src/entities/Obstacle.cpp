#include "../../include/entities/Obstacle.hpp"

Obstacle::Obstacle(std::string Name, Vector3D Position, Vector3D Normal, Material Mat)
    : Name(Name), Position(Position), Normal(Normal.Normalize()), ObstacleMaterial(Mat) {}

std::string Obstacle::GetName() const { return Name; }
Vector3D Obstacle::GetVelocity() const { return Vector3D(0, 0, 0); }
Material Obstacle::GetMaterial() const { return ObstacleMaterial; }
double Obstacle::GetRcs() const { return 10000.0; }
Vector3D Obstacle::GetPosition() const { return Position; }

void Obstacle::UpdatePhysics(double DeltaTime) {}

std::optional<HitRecord> Obstacle::CalculateIntersection(const ComplexWave& Wave) const {
    return IntersectionMath::RayIntersectPlane(Wave.Position, Wave.Direction, Position, Normal);
}