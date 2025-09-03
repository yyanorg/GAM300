/*********************************************************************************
* @File			Vector3.h 
* @Author		Ernest Ho, h.yonghengernest@digipen.edu
* @Co-Author	-
* @Date			3/9/2025
* @Brief		This is the Declaration of Vector3 Class
* 
* Copyright (C) 20xx DigiPen Institute of Technology. Reproduction or disclosure
* of this file or its contents without the prior written consent of DigiPen 
* Institute of Technology is prohibited. 
*********************************************************************************/

#pragma once

#include "pch.h"

class Vector3
{
private:

public:
	float x, y, z;

	// Constructs
	constexpr Vector3() : x(0.f), y(0.f), z(0.f) {}
	constexpr Vector3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

	//  Special helpers
	static constexpr Vector3 Zero() { return { 0.f,0.f,0.f }; }
	static constexpr Vector3 YAxis() { return { 0.f,1.f,0.f }; }
	static constexpr Vector3 ZAxis() { return { 0.f,0.f,1.f }; }
	static constexpr Vector3 Ones() { return { 1.f,1.f,1.f }; }
	static constexpr Vector3 XAxis() { return { 1.f,0.f,0.f }; }


	// Indexing
	float& operator[](int i);

	const float& operator[](int i) const;

	// Arithmetric
	constexpr Vector3 operator+() const noexcept { return *this; }
	constexpr Vector3 operator-() const noexcept { return { -x,-y,-z }; }

	Vector3 operator+(const Vector3&) const;
	Vector3 operator-(const Vector3&) const;
	Vector3 operator*(const Vector3&) const;
	Vector3 operator/(const Vector3&) const;

	Vector3 operator*(float) const;
	Vector3 operator/(float) const;

	Vector3& operator+=(const Vector3&);
	Vector3& operator-=(const Vector3&);
	Vector3& operator*=(const Vector3&);
	Vector3& operator/=(const Vector3&);

	Vector3& operator*=(float);
	Vector3& operator/=(float);

	// Comparison
	bool operator==(const Vector3&) const;
	bool operator!=(const Vector3&) const;

	// Math functions
	float dot(const Vector3&) const;
	Vector3 cross(const Vector3&) const;

	float length_sq() const;
	float length() const;

	Vector3 normalized() const;
	Vector3& normalize();

	Vector3 project_onto(const Vector3&) const;
	Vector3 reflect(const Vector3& normalized) const;

	static Vector3 Lerp(const Vector3& a, const Vector3& b, float t);

}v3;

// Left scalar
Vector3 operator*(float scalar, Vector3& v);

// Display Vector
std::ostream& operator<<(std::ostream& os, const Vector3& v);
