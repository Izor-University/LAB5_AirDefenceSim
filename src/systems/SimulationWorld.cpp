#include <cmath>
#include "../../include/systems/SimulationWorld.hpp"
#include "../../include/physics/WaveEngine.hpp"

SimulationWorld::SimulationWorld(int MaxBounces) : MaxBounces(MaxBounces) {}

void SimulationWorld::AddObject(std::shared_ptr<IPhysicalObject> Obj) {
    SpaceObjects.push_back(Obj);
}

void SimulationWorld::AddRadar(std::shared_ptr<Radar> Rdr) {
    Radars.push_back(Rdr);
}

void SimulationWorld::UpdatePhysics(double DeltaTime) {
    for (auto& Obj : SpaceObjects) {
        Obj->UpdatePhysics(DeltaTime);
    }
}

std::optional<NearestHit> SimulationWorld::FindNearestIntersection(const ComplexWave& Wave) const {
    std::optional<NearestHit> Nearest = std::nullopt;
    for (const auto& Obj : SpaceObjects) {
        auto HitOpt = Obj->CalculateIntersection(Wave);
        if (HitOpt.has_value()) {
            if (!Nearest.has_value() || HitOpt->Distance < Nearest->Distance) {
                Nearest = NearestHit{ HitOpt->Distance, Obj, HitOpt.value() };
            }
        }
    }
    return Nearest;
}

void SimulationWorld::ProcessRadarNetwork(const std::vector<Vector3D>& /*ScanDirections - больше не нужно*/) {
    DebugRays.clear();
    std::vector<Vector3D> GlobalTracks; // Все цели, обнаруженные сетью ПВО

    for (const auto& CurrentRadar : Radars) {
        // Радар сам знает, куда он сейчас повернут
        auto ActiveWaves = CurrentRadar->GenerateScanRays(20); // 5 лучей на сектор

        std::vector<ComplexWave> SuccessfulEchoes;

        for (auto& CurrentWave : ActiveWaves) {
            int BounceCount = 0;
            while (BounceCount < MaxBounces) {
                auto Nearest = FindNearestIntersection(CurrentWave);
                if (!Nearest.has_value()) {
                    if (BounceCount == 0) {
                        // Рисуем луч радара до MaxRange
                        Vector3D EndPoint = CurrentWave.Position + (CurrentWave.Direction * CurrentRadar->GetMaxRange());
                        DebugRays.push_back({CurrentWave.Position, EndPoint, false});
                    }
                    break;
                }

                DebugRays.push_back({CurrentWave.Position, Nearest->Record.HitPoint, BounceCount > 0});

                try {
                    ComplexWave ReflectedWave = WaveEngine::CalculateReflection(CurrentWave, Nearest->Record, *Nearest->Object);
                    Vector3D DirToRadar = (CurrentRadar->GetPosition() - ReflectedWave.Position).Normalize();
                    ComplexWave EchoWave = ReflectedWave;
                    EchoWave.Direction = DirToRadar;

                    auto BlockCheck = FindNearestIntersection(EchoWave);
                    if (!BlockCheck.has_value() || BlockCheck->Distance >= (CurrentRadar->GetPosition() - EchoWave.Position).Length()) {
                        SuccessfulEchoes.push_back(EchoWave);
                    }
                    CurrentWave = ReflectedWave;
                    BounceCount++;
                } catch (const PhysicsException& e) { break; }
            }
        }

        // Передаем эхо в радар и получаем обнаруженные координаты
        for (const auto& Echo : SuccessfulEchoes) {
            CurrentRadar->ReceiveEcho(Echo);
        }

        // Data Fusion: собираем треки со всех радаров
        auto LocalTracks = CurrentRadar->ProcessEchoesAndDetect();
        GlobalTracks.insert(GlobalTracks.end(), LocalTracks.begin(), LocalTracks.end());
    }

    // Сохраняем объединенные треки в публичном поле (нужно добавить его в .hpp)
    NetworkTracks = GlobalTracks;
}