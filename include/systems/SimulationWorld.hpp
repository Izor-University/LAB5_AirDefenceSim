#pragma once
#include <vector>
#include <memory>
#include <optional>
#include <algorithm>

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

// ДОБАВЛЕНО: Объединенный трек Командного Пункта (Data Fusion)
struct FusedTrack {
    int Id;                 // Стабильный номер цели
    Vector3D Position;      // Усредненная позиция
    double DangerVelocity;  // Максимальная радиальная скорость
    int SensorsTracking;    // Сколько радаров видят цель прямо сейчас
    int TimeSinceUpdate;    // Таймер потери
};

class SimulationWorld {
private:
    std::vector<std::shared_ptr<IPhysicalObject>> SpaceObjects;
    std::vector<std::shared_ptr<Radar>> Radars;
    int MaxBounces;

    std::vector<RaySegment> DebugRays;

    // ДОБАВЛЕНО: Хранилище стабильных треков и генератор ID
    std::vector<FusedTrack> ActiveTracks;
    int NextTrackId;

    std::optional<NearestHit> FindNearestIntersection(const ComplexWave& Wave) const;

public:
    SimulationWorld(int MaxBounces = 3);

    void AddObject(std::shared_ptr<IPhysicalObject> Obj);
    void AddRadar(std::shared_ptr<Radar> Rdr);

    const std::vector<RaySegment>& GetDebugRays() const { return DebugRays; }
    void ClearDebugRays() { DebugRays.clear(); }

    // ИЗМЕНЕНО: Возвращает объединенные треки
    const std::vector<FusedTrack>& GetTracks() const { return ActiveTracks; }

    void ClearTargets();

    void UpdatePhysics(double DeltaTime);
    void ProcessRadarNetwork(const std::vector<Vector3D>& ScanDirections);
};