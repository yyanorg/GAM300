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

#include "Math/Vector3D.h"

// Indexing
float& Vector3D::operator[](int i)
{
	if (i < 0 || i>2) throw std::out_of_range("Vector3 indexing out of range");
	return *(&x + i);
}

const float& Vector3D::operator[](int i) const
{
	if (i < 0 || i>2) throw std::out_of_range("Vector3 indexing out of range");
	return *(&x + i);
}

// Copy Constructor
Vector3D::Vector3D(const Vector3D& rhs)
{
	x = rhs.x;
	y = rhs.y;
}

Vector3D& Vector3D::operator=(const Vector3D& rhs)
{
	x = rhs.x;
	y = rhs.y;
	return *this;
}

// Overloading Operators
Vector3D Vector3D::operator+(const Vector3D& rhs) const { return { x + rhs.x, y + rhs.y, z + rhs.z }; }
Vector3D Vector3D::operator-(const Vector3D& rhs) const { return { x - rhs.x, y - rhs.y, z - rhs.z }; }
Vector3D Vector3D::operator*(const Vector3D& rhs) const { return { x * rhs.x, y * rhs.y, z * rhs.z }; }
Vector3D Vector3D::operator/(const Vector3D& rhs) const { return { x / rhs.x, y / rhs.y, z / rhs.z }; }

Vector3D Vector3D::operator*(float scalar) const { return { x * scalar, y * scalar, z * scalar }; }
Vector3D Vector3D::operator/(float scalar) const { return { x / scalar, y / scalar, z / scalar }; }

Vector3D& Vector3D::operator+=(const Vector3D& rhs)
{
	x += rhs.x;	y += rhs.y;	z += rhs.z;	
	return *this;
}

Vector3D& Vector3D::operator-=(const Vector3D& rhs)
{
	x -= rhs.x;	y -= rhs.y;	z -= rhs.z;
	return *this;
}

Vector3D& Vector3D::operator*=(const Vector3D& rhs)
{
	x *= rhs.x;	y *= rhs.y;	z *= rhs.z;
	return *this;
}

Vector3D& Vector3D::operator/=(const Vector3D& rhs)
{
	x /= rhs.x;	y /= rhs.y;	z /= rhs.z;
	return *this;
}

Vector3D& Vector3D::operator*=(float scalar)
{
	x *= scalar;	y *= scalar;	z *= scalar;
	return *this;
}

Vector3D& Vector3D::operator/=(float scalar)
{
	x /= scalar;	y /= scalar;	z /= scalar;
	return *this;
}


// Comparison
bool Vector3D::operator==(const Vector3D& rhs) const { return x == rhs.x && y == rhs.y && z == rhs.z; }
bool Vector3D::operator!=(const Vector3D& rhs) const { return !(*this == rhs); }


// Math functions
float Vector3D::dot(const Vector3D& rhs) const { return x * rhs.x + y * rhs.y + z * rhs.z; }

Vector3D Vector3D::cross(const Vector3D& rhs) const
{
	return
	{
		y * rhs.z - z * rhs.y,
		rhs.x * z - x * rhs.z,
		x * rhs.y - y * rhs.x
	};
}

float Vector3D::length_sq() const { return x * x + y * y + z * z; }
float Vector3D::length() const { return std::sqrt(length_sq()); }

Vector3D Vector3D::normalized() const 
{
	float len = length();
	return (len > 0.0f) ? (*this / len) : Vector3D::Zero();
}

Vector3D& Vector3D::normalize() 
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

Vector3D Vector3D::project_onto(const Vector3D& n) const 
{
	float d = n.length_sq();
	if (d <= 0.0f) return Vector3D::Zero();
	return n * (dot(n) / d);
}

Vector3D Vector3D::reflect(const Vector3D& n_normalized) const 
{
	return *this - n_normalized * (2.0f * this->dot(n_normalized));
}

Vector3D Vector3D::Lerp(const Vector3D& a, const Vector3D& b, float t) 
{
	return a + (b - a) * t;
}

// ---- Scalar left ----
Vector3D operator*(float scalar, const Vector3D& v) { return v * scalar; }

// ---- Stream output ----
std::ostream& operator<<(std::ostream& os, const Vector3D& v) {
	return os << '(' << v.x << ", " << v.y << ", " << v.z << ')';
}