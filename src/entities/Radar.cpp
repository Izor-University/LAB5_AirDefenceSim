#include "../../include/entities/Radar.hpp"
#include <cmath>

Radar::Radar(std::string Id, Vector3D Position, double CarrierFrequency)
    : Id(Id), Position(Position), CarrierFrequency(CarrierFrequency),
      SweepDirection(1), RotationSpeed(2.0), BeamWidth(0.15), MaxRange(25000.0),
      Mode(RadarMode::SCANNING), TrackLossCounter(0), SmoothedVelocity(0.0)
{
    SetSector(0, 360);
}

std::string Radar::GetId() const { return Id; }
Vector3D Radar::GetPosition() const { return Position; }
double Radar::GetMaxRange() const { return MaxRange; }
RadarMode Radar::GetMode() const { return Mode; }
double Radar::GetAzimuth() const { return CurrentAzimuth; }

void Radar::SetSector(double MinDeg, double MaxDeg) {
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
        Vector3D DirToTarget = LockedTargetPos - Position;
        double TargetAzimuth = std::atan2(DirToTarget.Y, DirToTarget.X);
        double Distance = DirToTarget.Length();

        // Приводим угол цели к диапазону нашего сектора (нормализация углов)
        while (TargetAzimuth < MinAzimuth - 3.14159) TargetAzimuth += 2 * 3.14159;
        while (TargetAzimuth > MaxAzimuth + 3.14159) TargetAzimuth -= 2 * 3.14159;

        // ИСПРАВЛЕНИЕ: Если цель вылетела за механические пределы радара или слишком далеко
        if (TargetAzimuth < MinAzimuth || TargetAzimuth > MaxAzimuth || Distance > MaxRange) {
            // Сбрасываем захват (цель "ушла" из зоны обзора)
            Mode = RadarMode::SCANNING;
            TrackLossCounter = 0;
            BeamWidth = 0.15;
            SmoothedVelocity = 0.0;
        } else {
            // Продолжаем вести цель
            CurrentAzimuth = TargetAzimuth;
        }
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

void Radar::ReceiveEcho(const ComplexWave& Echo) {
    ReceivedEchoes.push_back(Echo);
}

std::vector<RadarTrack> Radar::ProcessEchoesAndDetect() {
    DetectedTargets.clear();
    bool TargetFoundThisFrame = false;
    const double C = 299792458.0;

    for (const auto& Echo : ReceivedEchoes) {
        if (std::abs(Echo.Amplitude) > 1e-15) {
            double DopplerShift = Echo.Frequency - CarrierFrequency;
            double CalculatedVelocity = -(DopplerShift / CarrierFrequency) * (C / 2.0);

            // MTI Фильтр (СДЦ): Игнорируем всё, чья радиальная скорость меньше 30 м/с!
            if (std::abs(CalculatedVelocity) > 30.0) {

                // Кластеризация эхо-сигналов (Слияние точек)
                bool Merged = false;
                for (auto& Track : DetectedTargets) {
                    if ((Track.Position - Echo.Position).Length() < 400.0) {
                        Track.RadialVelocity = (Track.RadialVelocity + CalculatedVelocity) / 2.0;
                        Track.Position = (Track.Position + Echo.Position) * 0.5;
                        Merged = true;
                        break;
                    }
                }

                if (!Merged) {
                    // EMA Сглаживание скорости для стабильного вывода на дисплей
                    if (SmoothedVelocity == 0.0) {
                        SmoothedVelocity = CalculatedVelocity;
                    } else {
                        SmoothedVelocity = SmoothedVelocity * 0.85 + CalculatedVelocity * 0.15;
                    }

                    RadarTrack Track;
                    Track.Position = Echo.Position;
                    Track.RadialVelocity = SmoothedVelocity;
                    DetectedTargets.push_back(Track);
                }

                LockedTargetPos = Echo.Position;
                TargetFoundThisFrame = true;
            }
        }
    }
    ReceivedEchoes.clear();

    // Конечный автомат радара
    if (TargetFoundThisFrame) {
        Mode = RadarMode::TRACKING;
        TrackLossCounter = 0;
        BeamWidth = 0.4;
    } else {
        if (Mode == RadarMode::TRACKING) {
            TrackLossCounter++;

            // Если цель исчезла (спряталась за гору или в слепую зону) на 0.25 сек
            if (TrackLossCounter > 15) {
                Mode = RadarMode::SCANNING;
                BeamWidth = 0.15;
                SmoothedVelocity = 0.0;
            }
        }
    }

    LastTracks = DetectedTargets;
    return DetectedTargets;
}