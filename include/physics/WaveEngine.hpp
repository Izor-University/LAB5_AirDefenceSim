#pragma once
#include "../core/ComplexWave.hpp"
#include "../core/IntersectionMath.hpp"
#include "IPhysicalObject.hpp"
#include <stdexcept>

class PhysicsException : public std::runtime_error {
public:
    explicit PhysicsException(const std::string& Message) : std::runtime_error(Message) {}
};

namespace WaveEngine {
    const double SpeedOfLight = 299792458.0;
    const double Pi = 3.14159265358979323846;

    ComplexWave CalculateReflection(const ComplexWave& IncomingWave, const HitRecord& Hit, const IPhysicalObject& HitObject);
}