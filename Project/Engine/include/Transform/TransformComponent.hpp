#pragma once
#include "Math/Matrix4x4.hpp"

struct Transform {
	Vector3D position = { 0, 0, 0 };
	Vector3D scale = { 1, 1, 1 };
	Vector3D rotation = { 0, 0, 0 };

	Vector3D lastPosition = { 0, 0, 0 };
	Vector3D lastScale = { 0, 0, 0 };
	Vector3D lastRotation = { 0, 0, 0 };

	Matrix4x4 model{};

	Transform() = default;
	~Transform() = default;
};