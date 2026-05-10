#pragma once
#include "../core/Vector3D.hpp"
#include "../core/ComplexWave.hpp"
#include <vector>
#include <string>

enum class RadarMode {
    SCANNING, // Поиск (секторное сканирование)
    TRACKING  // Автосопровождение захваченной цели
};

class Radar {
private:
    std::string Id;
    Vector3D Position;
    double CarrierFrequency;

    // Параметры сектора сканирования
    double MinAzimuth;
    double MaxAzimuth;
    int SweepDirection; // Направление вращения (1 или -1)

    double CurrentAzimuth;
    double RotationSpeed;
    double BeamWidth;
    double MaxRange;

    // Автосопровождение
    RadarMode Mode;
    Vector3D LockedTargetPos;
    int TrackLossCounter; // Таймер потери цели (чтобы вернуться к сканированию)

    std::vector<ComplexWave> ReceivedEchoes;
    std::vector<Vector3D> DetectedTargets;

public:
    Radar(std::string Id, Vector3D Position, double CarrierFrequency);

    std::string GetId() const;
    Vector3D GetPosition() const;
    double GetMaxRange() const;
    RadarMode GetMode() const;

    // Установка сектора обзора (в градусах)
    void SetSector(double MinDeg, double MaxDeg);
    void SetRotationSpeed(double Speed);

    void UpdatePhysics(double DeltaTime);

    std::vector<ComplexWave> GenerateScanRays(int RayCount) const;
    void ReceiveEcho(const ComplexWave& Echo);
    std::vector<Vector3D> ProcessEchoesAndDetect();
};