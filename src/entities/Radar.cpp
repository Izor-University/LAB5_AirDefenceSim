#include "../../include/entities/Radar.hpp"
#include <cmath>

Radar::Radar(std::string Id, Vector3D Position, double CarrierFrequency)
    : Id(Id), Position(Position), CarrierFrequency(CarrierFrequency),
      SweepDirection(1), RotationSpeed(2.0), BeamWidth(0.15), MaxRange(25000.0),
      Mode(RadarMode::SCANNING), TrackLossCounter(0)
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

            // ИЗМЕНЕНИЕ 1: Увеличиваем размер Слепой Зоны Доплера до 30 м/с!
            if (std::abs(CalculatedVelocity) > 30.0) {

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
                    RadarTrack Track;
                    Track.Position = Echo.Position;
                    Track.RadialVelocity = CalculatedVelocity;
                    DetectedTargets.push_back(Track);
                }

                LockedTargetPos = Echo.Position;
                TargetFoundThisFrame = true;
            }
        }
    }
    ReceivedEchoes.clear();

    if (TargetFoundThisFrame) {
        Mode = RadarMode::TRACKING;
        TrackLossCounter = 0;
        BeamWidth = 0.4;
    } else {
        if (Mode == RadarMode::TRACKING) {
            TrackLossCounter++;

            // ИЗМЕНЕНИЕ 2: Сокращаем память радара до 15 кадров (0.25 секунды).
            // Если ракета пропадает из Доплера, радар мгновенно её теряет!
            if (TrackLossCounter > 15) {
                Mode = RadarMode::SCANNING;
                BeamWidth = 0.15;
            }
        }
    }
    LastTracks = DetectedTargets;
    return DetectedTargets;
}