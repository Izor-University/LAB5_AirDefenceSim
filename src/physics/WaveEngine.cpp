#include "../../include/physics/WaveEngine.hpp"
#include <cmath>

namespace WaveEngine {
    ComplexWave CalculateReflection(const ComplexWave& IncomingWave, const HitRecord& Hit, const IPhysicalObject& HitObject) {
        if (Hit.Distance <= 0.0) {
            throw PhysicsException("Ошибка физики: Дистанция до объекта меньше или равна нулю.");
        }

        Vector3D OutgoingDirection = IncomingWave.Direction.Reflect(Hit.Normal);
        Vector3D Velocity = HitObject.GetVelocity();
        double V_in = Velocity.DotProduct(IncomingWave.Direction);
        double V_out = Velocity.DotProduct(OutgoingDirection);
        
        double DopplerShiftFactor = 1.0 - ((V_in - V_out) / SpeedOfLight);
        double NewFrequency = IncomingWave.Frequency * DopplerShiftFactor;

        double Distance = Hit.Distance;
        double Rcs = HitObject.GetRcs();
        double Attenuation = std::sqrt(Rcs) / Distance; 

        double WaveNumber = (2.0 * Pi * IncomingWave.Frequency) / SpeedOfLight;
        double PhaseRotation = WaveNumber * Distance;
        
        Material ObjMaterial = HitObject.GetMaterial();
        PhaseRotation += ObjMaterial.PhaseShift;

        std::complex<double> WaveMultiplier = std::polar(Attenuation * ObjMaterial.Reflectivity, PhaseRotation);
        std::complex<double> NewAmplitude = IncomingWave.Amplitude * WaveMultiplier;

        if (std::abs(NewAmplitude) < 1e-15) {
            throw PhysicsException("Сигнал затух ниже порога чувствительности.");
        }

        ComplexWave ReflectedWave(Hit.HitPoint, OutgoingDirection, NewFrequency, NewAmplitude);
        ReflectedWave.DistanceTraveled = IncomingWave.DistanceTraveled + Distance;

        return ReflectedWave;
    }
}