#pragma once
#include "Vector3D.hpp"
#include <complex>

struct ComplexWave {
    Vector3D Position;
    Vector3D Direction;
    double Frequency;
    std::complex<double> Amplitude;
    double DistanceTraveled;

    ComplexWave(Vector3D Pos, Vector3D Dir, double Freq, std::complex<double> Amp)
        : Position(Pos), Direction(Dir.Normalize()), Frequency(Freq), Amplitude(Amp), DistanceTraveled(0.0) {}
};