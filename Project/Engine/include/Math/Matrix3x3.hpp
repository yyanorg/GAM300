/*********************************************************************************
* @File			Matrix3x3.h
* @Author		Ernest Ho, h.yonghengernest@digipen.edu
* @Co-Author	-
* @Date			3/9/2025
* @Brief		This is the Declaration of 3x3 Matrix Class
*
* Copyright (C) 20xx DigiPen Institute of Technology. Reproduction or disclosure
* of this file or its contents without the prior written consent of DigiPen
* Institute of Technology is prohibited.
*********************************************************************************/
#pragma once

#include "pch.h"
#include "Math/Vector3D.hpp"

struct Matrix3x3 {
    // Storage: row-major (m[row][col])
    float m[3][3];

    // ---- ctors ----
    Matrix3x3();                                 // identity by default is convenient
    Matrix3x3(float m00, float m01, float m02,
        float m10, float m11, float m12,
        float m20, float m21, float m22);

    // ---- element access ----
    float* operator[](int r) { return m[r]; }
    const float* operator[](int r) const { return m[r]; }

    // ---- arithmetic ----
    Matrix3x3  operator+(const Matrix3x3& rhs) const;
    Matrix3x3  operator-(const Matrix3x3& rhs) const;
    Matrix3x3  operator*(const Matrix3x3& rhs) const;     // composition
    Matrix3x3& operator*=(const Matrix3x3& rhs);
    Matrix3x3  operator*(float s) const;
    Matrix3x3  operator/(float s) const;
    Matrix3x3& operator*=(float s);
    Matrix3x3& operator/=(float s);

    Vector3D   operator*(const Vector3D& v) const;        // M * v

    bool operator==(const Matrix3x3& rhs) const;

    // ---- linear algebra ----
    float     determinant() const;
    Matrix3x3 cofactor() const;
    Matrix3x3 transposed() const;
    bool      tryInverse(Matrix3x3& out) const;           // false if singular
    Matrix3x3 inversed() const;                            // may assert/throw if singular

    // ---- factories (preferred over mutating setters) ----
    static Matrix3x3 identity();
    static Matrix3x3 zero();
    static Matrix3x3 scale(float sx, float sy, float sz);
    static Matrix3x3 rotateX(float radians);
    static Matrix3x3 rotateY(float radians);
    static Matrix3x3 rotateZ(float radians);
    static Matrix3x3 rotationAxisAngle(const Vector3D& axis_unit, float radians); // optional
};

typedef Matrix3x3 Mat3;

// left scalar
inline Matrix3x3 operator*(float s, const Matrix3x3& M) { return M * s; }

// Output
std::ostream& operator<<(std::ostream& os, const Matrix3x3& mat);