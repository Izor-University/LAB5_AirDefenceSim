#pragma once

struct Material {
    double Reflectivity;
    double PhaseShift;

    Material(double Reflectivity = 1.0, double PhaseShift = 0.0)
        : Reflectivity(Reflectivity), PhaseShift(PhaseShift) {}
};