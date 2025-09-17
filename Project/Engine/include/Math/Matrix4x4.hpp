/*********************************************************************************
* @File			Matrix4x4.hpp
* @Author		Ernest Ho, h.yonghengernest@digipen.edu
* @Co-Author	-
* @Date			3/9/2025
* @Brief		This is the Declaration of 4x4 Matrix Class
*
* Copyright (C) 20xx DigiPen Institute of Technology. Reproduction or disclosure
* of this file or its contents without the prior written consent of DigiPen
* Institute of Technology is prohibited.
*********************************************************************************/
#pragma once

#include "pch.h"
#include "Math/Vector3D.hpp"

struct Matrix4x4 {
    // Row-major storage: m[row][col]
    float m[4][4];

    // ---- ctors ----
    Matrix4x4(); // identity
    Matrix4x4(float m00, float m01, float m02, float m03,
        float m10, float m11, float m12, float m13,
        float m20, float m21, float m22, float m23,
        float m30, float m31, float m32, float m33);

    // ---- element access ----
    float* operator[](int r) { return m[r]; }
    const float* operator[](int r) const { return m[r]; }

    // ---- arithmetic ----
    Matrix4x4  operator+(const Matrix4x4& rhs) const;
    Matrix4x4  operator-(const Matrix4x4& rhs) const;
    Matrix4x4  operator*(const Matrix4x4& rhs) const; // composition
    Matrix4x4& operator*=(const Matrix4x4& rhs);
    Matrix4x4  operator*(float s) const;
    Matrix4x4  operator/(float s) const;
    Matrix4x4& operator*=(float s);
    Matrix4x4& operator/=(float s);

    bool operator==(const Matrix4x4& rhs) const;

    // ---- vector transforms (column-vector convention) ----
    // Treats v as (x,y,z,1). Returns perspective-divided Vector3D.
    Vector3D TransformPoint(const Vector3D& v) const;
    // Treats v as (x,y,z,0). Ignores translation.
    Vector3D TransformVector(const Vector3D& v) const;

    // ---- linear algebra ----
    Matrix4x4 Transposed() const;
    float     Determinant() const;
    bool      TryInverse(Matrix4x4& out) const;  // false if singular
    Matrix4x4 Inversed() const;                  // asserts if singular

    // ---- factories ----
    static Matrix4x4 Identity();
    static Matrix4x4 Zero();

    static Matrix4x4 Translate(float tx, float ty, float tz);
    static Matrix4x4 Scale(float sx, float sy, float sz);
    static Matrix4x4 Scale(float s) { return Scale(s, s, s); }

    static Matrix4x4 RotationX(float radians);
    static Matrix4x4 RotationY(float radians);
    static Matrix4x4 RotationZ(float radians);
    static Matrix4x4 RotationAxisAngle(const Vector3D& axis_unit, float radians);

    // Compose: T * R * S (column-vector math; applied S then R then T)
    static Matrix4x4 TRS(const Vector3D& t, const Matrix4x4& R, const Vector3D& s);

    // ---- camera / projection (Right-Handed) ----
    static Matrix4x4 LookAtRH(const Vector3D& eye, const Vector3D& target, const Vector3D& up);

    // fovY in radians, aspect = width/height, zNear>0, zFar>zNear
    static Matrix4x4 PerspectiveFovRH(float fovY, float aspect, float zNear, float zFar);
    static Matrix4x4 OrthoRH(float left, float right, float bottom, float top, float zNear, float zFar);
};

// left scalar
inline Matrix4x4 operator*(float s, const Matrix4x4& M) { return M * s; }

// Output
std::ostream& operator<<(std::ostream& os, const Matrix4x4& mat);