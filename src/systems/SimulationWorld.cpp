#include "../../include/systems/SimulationWorld.hpp"
#include "../../include/physics/WaveEngine.hpp"

SimulationWorld::SimulationWorld(int MaxBounces) : MaxBounces(MaxBounces), NextTrackId(1) {}

void SimulationWorld::AddObject(std::shared_ptr<IPhysicalObject> Obj) { SpaceObjects.push_back(Obj); }
void SimulationWorld::AddRadar(std::shared_ptr<Radar> Rdr) { Radars.push_back(Rdr); }

void SimulationWorld::UpdatePhysics(double DeltaTime) {
    for (auto& Obj : SpaceObjects) Obj->UpdatePhysics(DeltaTime);
}

void SimulationWorld::ClearTargets() {
    auto it = SpaceObjects.begin();
    while (it != SpaceObjects.end()) {
        if ((*it)->GetName().find("Target") != std::string::npos) {
            it = SpaceObjects.erase(it);
        } else {
            ++it;
        }
    }
    // ИСПРАВЛЕНИЕ: Полностью стираем память Командного Центра при удалении
    ActiveTracks.clear();
    DebugRays.clear();
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

void SimulationWorld::ProcessRadarNetwork(const std::vector<Vector3D>& /*ScanDirections*/) {
    DebugRays.clear();

    for (auto& Track : ActiveTracks) {
        Track.TimeSinceUpdate++;
        Track.SensorsTracking = 0;
    }

    for (const auto& CurrentRadar : Radars) {
        auto ActiveWaves = CurrentRadar->GenerateScanRays(60);
        std::vector<ComplexWave> SuccessfulEchoes;

        for (auto& CurrentWave : ActiveWaves) {
            int BounceCount = 0;
            while (BounceCount < MaxBounces) {
                auto Nearest = FindNearestIntersection(CurrentWave);
                if (!Nearest.has_value()) {
                    if (BounceCount == 0) {
                        Vector3D EndPoint = CurrentWave.Position + (CurrentWave.Direction * CurrentRadar->GetMaxRange());
                        DebugRays.push_back({CurrentWave.Position, EndPoint, false});
                    }
                    break;
                }

                DebugRays.push_back({CurrentWave.Position, Nearest->Record.HitPoint, BounceCount > 0});

                try {
                    ComplexWave ReflectedWave = WaveEngine::CalculateReflection(CurrentWave, Nearest->Record, *Nearest->Object);
                    if (Nearest->Object->GetName() != "Ground") {
                        Vector3D DirToRadar = (CurrentRadar->GetPosition() - ReflectedWave.Position).Normalize();
                        Vector3D Velocity = Nearest->Object->GetVelocity();
                        double V_in = Velocity.DotProduct(CurrentWave.Direction);
                        double V_out = Velocity.DotProduct(DirToRadar);
                        double TrueDopplerShift = 1.0 - ((V_in - V_out) / WaveEngine::SpeedOfLight);

                        ComplexWave EchoWave = ReflectedWave;
                        EchoWave.Direction = DirToRadar;
                        EchoWave.Frequency = CurrentWave.Frequency * TrueDopplerShift;

                        auto BlockCheck = FindNearestIntersection(EchoWave);
                        if (!BlockCheck.has_value() || BlockCheck->Distance >= (CurrentRadar->GetPosition() - EchoWave.Position).Length()) {
                            SuccessfulEchoes.push_back(EchoWave);
                        }
                    }
                    CurrentWave = ReflectedWave;
                    BounceCount++;
                } catch (const PhysicsException& e) { break; }
            }
        }

        for (const auto& Echo : SuccessfulEchoes) CurrentRadar->ReceiveEcho(Echo);

        auto LocalTracks = CurrentRadar->ProcessEchoesAndDetect();

        for (const auto& LT : LocalTracks) {
            bool Matched = false;
            for (auto& FT : ActiveTracks) {
                if ((FT.Position - LT.Position).Length() < 600.0) {

                    // ИСПРАВЛЕНИЕ: Мгновенная привязка рамки к цели (убирает отставание фантома!)
                    FT.Position = LT.Position;
                    FT.DangerVelocity = std::max(FT.DangerVelocity, std::abs(LT.RadialVelocity));
                    FT.SensorsTracking++;
                    FT.TimeSinceUpdate = 0;
                    Matched = true;
                    break;
                }
            }
            if (!Matched) {
                ActiveTracks.push_back({NextTrackId++, LT.Position, std::abs(LT.RadialVelocity), 1, 0});
            }
        }
    }

    // ИСПРАВЛЕНИЕ: Уменьшаем время "забывания" до 60 кадров (1 секунда).
    // Зависшие в воздухе фантомы будут исчезать очень быстро.
    ActiveTracks.erase(
        std::remove_if(ActiveTracks.begin(), ActiveTracks.end(),[](const FusedTrack& t) { return t.TimeSinceUpdate > 60; }
        ),
        ActiveTracks.end()
    );
}