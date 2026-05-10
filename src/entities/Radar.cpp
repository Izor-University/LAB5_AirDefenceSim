#include "../../include/entities/Radar.hpp"
#include <cmath>

Radar::Radar(std::string Id, Vector3D Position, double CarrierFrequency)
    : Id(Id), Position(Position), CarrierFrequency(CarrierFrequency),
      SweepDirection(1), RotationSpeed(2.0), BeamWidth(0.15), MaxRange(25000.0),
      Mode(RadarMode::SCANNING), TrackLossCounter(0)
{
    SetSector(0, 360); // По умолчанию крутится вокруг
}

std::string Radar::GetId() const { return Id; }
Vector3D Radar::GetPosition() const { return Position; }
double Radar::GetMaxRange() const { return MaxRange; }
RadarMode Radar::GetMode() const { return Mode; }

void Radar::SetSector(double MinDeg, double MaxDeg) {
    // Перевод из градусов в радианы
    MinAzimuth = MinDeg * (3.14159265 / 180.0);
    MaxAzimuth = MaxDeg * (3.14159265 / 180.0);
    CurrentAzimuth = (MinAzimuth + MaxAzimuth) / 2.0;
}

void Radar::SetRotationSpeed(double Speed) { RotationSpeed = Speed; }

void Radar::UpdatePhysics(double DeltaTime) {
    if (Mode == RadarMode::SCANNING) {
        // Качание антенны в пределах сектора
        CurrentAzimuth += RotationSpeed * SweepDirection * DeltaTime;
        if (CurrentAzimuth >= MaxAzimuth) {
            CurrentAzimuth = MaxAzimuth;
            SweepDirection = -1;
        } else if (CurrentAzimuth <= MinAzimuth) {
            CurrentAzimuth = MinAzimuth;
            SweepDirection = 1;
        }
    }
    else if (Mode == RadarMode::TRACKING) {
        // АВТОСОПРОВОЖДЕНИЕ: Радар направлен точно на последнюю известную позицию цели
        Vector3D DirToTarget = LockedTargetPos - Position;
        CurrentAzimuth = std::atan2(DirToTarget.Y, DirToTarget.X);
    }
}

std::vector<ComplexWave> Radar::GenerateScanRays(int RayCount) const {
    std::vector<ComplexWave> Waves;
    double StartAngle = CurrentAzimuth - (BeamWidth / 2.0);
    double AngleStep = BeamWidth / (RayCount > 1 ? RayCount - 1 : 1);

    for (int i = 0; i < RayCount; ++i) {
        double Angle = StartAngle + (i * AngleStep);
        Vector3D Direction(std::cos(Angle), std::sin(Angle), 0.0);
        Waves.push_back(ComplexWave(Position, Direction, CarrierFrequency, std::complex<double>(1.0, 0.0)));
    }
    return Waves;
}

// === ВЕРНУЛИ ПОТЕРЯННЫЙ МЕТОД ===
void Radar::ReceiveEcho(const ComplexWave& Echo) {
    ReceivedEchoes.push_back(Echo);
}
// ================================

std::vector<Vector3D> Radar::ProcessEchoesAndDetect() {
    DetectedTargets.clear();
    bool TargetFoundThisFrame = false;

    for (const auto& Echo : ReceivedEchoes) {
        if (std::abs(Echo.Amplitude) > 1e-15) {

            // ДОПЛЕРОВСКИЙ ФИЛЬТР (MTI)
            if (std::abs(Echo.Frequency - CarrierFrequency) > 1.0) {
                DetectedTargets.push_back(Echo.Position);
                LockedTargetPos = Echo.Position;
                TargetFoundThisFrame = true;
            }
        }
    }
    ReceivedEchoes.clear();

    // Логика перехода между режимами
    if (TargetFoundThisFrame) {
        Mode = RadarMode::TRACKING;
        TrackLossCounter = 0;
    } else {
        if (Mode == RadarMode::TRACKING) {
            TrackLossCounter++;
            if (TrackLossCounter > 60) {
                Mode = RadarMode::SCANNING;
            }
        }
    }

    return DetectedTargets;
}