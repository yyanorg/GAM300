/*********************************************************************************
* @File			Vector3.cpp
* @Author		Ernest Ho, h.yonghengernest@digipen.edu
* @Co-Author	-
* @Date			3/9/2025
* @Brief		This is the Definition of Vector3 Class
*
* Copyright (C) 20xx DigiPen Institute of Technology. Reproduction or disclosure
* of this file or its contents without the prior written consent of DigiPen
* Institute of Technology is prohibited.
*********************************************************************************/

#include "Math/Vector3.h"

float& Vector3::operator[](int i)
{
	if (i < 0 || i>2) throw std::out_of_range("Vector3 indexing out of range");
	return *(&x + i);
}

const float& Vector3::operator[](int i) const
{
	if (i < 0 || i>2) throw std::out_of_range("Vector3 indexing out of range");
	return *(&x + i);
}

Vector3 Vector3::operator+(const Vector3& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
Vector3 Vector3::operator-(const Vector3& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
Vector3 Vector3::operator*(const Vector3& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
Vector3 Vector3::operator/(const Vector3& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

Vector3 Vector3::operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
Vector3 Vector3::operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }

Vector3& Vector3::operator+=(const Vector3& rhs)
{
	x += rhs.x;	y += rhs.y;	z += rhs.z;	
	return *this;
}

Vector3& Vector3::operator-=(const Vector3& rhs)
{
	x -= rhs.x;	y -= rhs.y;	z -= rhs.z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3& rhs)
{
	x *= rhs.x;	y *= rhs.y;	z *= rhs.z;
	return *this;
}

Vector3& Vector3::operator/=(const Vector3& rhs)
{
	x /= rhs.x;	y /= rhs.y;	z /= rhs.z;
	return *this;
}

Vector3& Vector3::operator*=(float scalar)
{
	x *= scalar;	y *= scalar;	z *= scalar;
	return *this;
}

Vector3& Vector3::operator/=(float scalar)
{
	x /= scalar;	y /= scalar;	z /= scalar;
	return *this;
}


// Comparison
bool Vector3::operator==(const Vector3& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool Vector3::operator!=(const Vector3& rhs) const { return !(*this == rhs); }


// Math functions
float Vector3::dot(const Vector3& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

Vector3 Vector3::cross(const Vector3& rhs) const
{
	return
	{
		y * rhs.z - z * rhs.y,
		rhs.x * z - x * rhs.z,
		x * rhs.y - y * rhs.x
	};
}

float Vector3::length_sq() const { return x * x + y * y + z * z; }
float Vector3::length() const { return std::sqrt(length_sq()); }

Vector3 Vector3::normalized() const 
{
	float len = length();
	return (len > 0.0f) ? (*this / len) : Vector3::Zero();
}

Vector3& Vector3::normalize() 
{
	float len = length();
	if (len > 0.0f) 
	{ 
		x /= len; 
		y /= len; 
		z /= len; 
	}
	return *this;
}

Vector3 Vector3::project_onto(const Vector3& n) const 
{
	float d = n.length_sq();
	if (d <= 0.0f) return Vector3::Zero();
	return n * (dot(n) / d);
}

Vector3 Vector3::reflect(const Vector3& n_normalized) const 
{
	return *this - n_normalized * (2.0f * this->dot(n_normalized));
}

Vector3 Vector3::Lerp(const Vector3& a, const Vector3& b, float t) 
{
	return a + (b - a) * t;
}

// ---- Scalar left ----
Vector3 operator*(float scalar, const Vector3& v) { return v * scalar; }

// ---- Stream output ----
std::ostream& operator<<(std::ostream& os, const Vector3& v) {
	return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}