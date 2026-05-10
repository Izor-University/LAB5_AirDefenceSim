#pragma once
#include <cmath>

struct Vector3D {
    double X;
    double Y;
    double Z;

    Vector3D(double X = 0.0, double Y = 0.0, double Z = 0.0) : X(X), Y(Y), Z(Z) {}

    inline Vector3D operator+(const Vector3D& Other) const { return Vector3D(X + Other.X, Y + Other.Y, Z + Other.Z); }
    inline Vector3D operator-(const Vector3D& Other) const { return Vector3D(X - Other.X, Y - Other.Y, Z - Other.Z); }
    inline Vector3D operator*(double Scalar) const { return Vector3D(X * Scalar, Y * Scalar, Z * Scalar); }
    inline double DotProduct(const Vector3D& Other) const { return X * Other.X + Y * Other.Y + Z * Other.Z; }

    inline double Length() const { return std::sqrt(X * X + Y * Y + Z * Z); }

    inline Vector3D Normalize() const {
        double Len = Length();
        if (Len > 0.0) return Vector3D(X / Len, Y / Len, Z / Len);
        return *this;
    }

    inline Vector3D Reflect(const Vector3D& Normal) const {
        return *this - (Normal * (2.0 * this->DotProduct(Normal)));
    }
};