#pragma once
#include "../core/Vector3D.hpp"
#include "../core/ComplexWave.hpp"
#include <vector>
#include <string>

struct RadarTrack {
    Vector3D Position;
    double RadialVelocity;
};

enum class RadarMode { SCANNING, TRACKING };

class Radar {
private:
    std::string Id;
    Vector3D Position;
    double CarrierFrequency;

    double MinAzimuth;
    double MaxAzimuth;
    int SweepDirection;

    double CurrentAzimuth;
    double RotationSpeed;
    double BeamWidth;
    double MaxRange;

    RadarMode Mode;
    Vector3D LockedTargetPos;
    int TrackLossCounter;

    double SmoothedVelocity;

    std::vector<ComplexWave> ReceivedEchoes;
    std::vector<RadarTrack> DetectedTargets;

    // ДОБАВЛЕНО: Сохраняем локальные цели, которые видит именно этот радар
    std::vector<RadarTrack> LastTracks;

public:
    Radar(std::string Id, Vector3D Position, double CarrierFrequency);

    std::string GetId() const;
    Vector3D GetPosition() const;
    double GetMaxRange() const;
    RadarMode GetMode() const;
    double GetAzimuth() const;

    // ДОБАВЛЕНО: Геттер для индивидуального дисплея
    const std::vector<RadarTrack>& GetLastTracks() const { return LastTracks; }

    void SetSector(double MinDeg, double MaxDeg);
    void SetRotationSpeed(double Speed);

    void UpdatePhysics(double DeltaTime);

    std::vector<ComplexWave> GenerateScanRays(int RayCount) const;
    void ReceiveEcho(const ComplexWave& Echo);
    std::vector<RadarTrack> ProcessEchoesAndDetect();
};