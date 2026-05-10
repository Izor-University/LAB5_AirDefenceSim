#pragma once
#include <vector>
#include <memory>
#include <optional>

#include "../physics/IPhysicalObject.hpp"
#include "../entities/Radar.hpp"

struct RaySegment {
    Vector3D Start;
    Vector3D End;
    bool IsBounce;
};

struct NearestHit {
    double Distance;
    std::shared_ptr<IPhysicalObject> Object;
    HitRecord Record;
};

class SimulationWorld {
private:
    std::vector<std::shared_ptr<IPhysicalObject>> SpaceObjects;
    std::vector<std::shared_ptr<Radar>> Radars;
    int MaxBounces;

    std::vector<RaySegment> DebugRays;
    std::vector<Vector3D> NetworkTracks; // ДОБАВЛЕНО: Хранилище треков сети ПВО

    std::optional<NearestHit> FindNearestIntersection(const ComplexWave& Wave) const;

public:
    SimulationWorld(int MaxBounces = 3);

    void AddObject(std::shared_ptr<IPhysicalObject> Obj);
    void AddRadar(std::shared_ptr<Radar> Rdr);

    const std::vector<RaySegment>& GetDebugRays() const { return DebugRays; }
    void ClearDebugRays() { DebugRays.clear(); }

    // ДОБАВЛЕНО: Геттер для получения треков
    const std::vector<Vector3D>& GetTracks() const { return NetworkTracks; }

    void UpdatePhysics(double DeltaTime);
    void ProcessRadarNetwork(const std::vector<Vector3D>& ScanDirections);
};