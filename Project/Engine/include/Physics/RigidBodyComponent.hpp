/*********************************************************************************
* @File			RigidBodyComponent.hpp
* @Author		Ang Jia Jun Austin, a.jiajunaustin@digipen.edu
* @Co-Author	-
* @Date			25/10/2025
* @Brief		Rigid body component for physics simulation. Defines physical
*				properties and motion behavior for entities in the physics system.
*
* Copyright (C) 2025 DigiPen Institute of Technology. Reproduction or disclosure
* of this file or its contents without the prior written consent of DigiPen
* Institute of Technology is prohibited.
*********************************************************************************/

#pragma once
#include "Physics/JoltInclude.hpp"
#include "pch.h"
#include "Math/Vector3D.hpp"
#include "Reflection/ReflectionBase.hpp"

enum class Motion : int { 
	Static = 0, 
	Kinematic, 
	Dynamic 
};

struct PhysicsMaterial {
	std::string name = "Default";
	float friction = 0.6f;
};



struct RigidBodyComponent {
	REFL_SERIALIZABLE
	bool enabled = true;          // Component enabled state (can be toggled in inspector)
	int motionID{};

	bool ccd		= false; // continuous collision detection
	bool isTrigger	= false;

	float gravityFactor = 1.0f;

	Vector3D angularVel = { 0.0f,0.0f,0.0f };
	Vector3D linearVel = { 0.0f, -9.81f,0.0f };

	float linearDamping = 0.0f;
	float angularDamping = 0.0f;

	bool isTeleporting = false;

	//TO BE USED FOR SCRIPT
	Vector3D forceApplied = { 0.0f,0.0f,0.0f };
	Vector3D torqueApplied = { 0.0f,0.0f,0.0f };
	Vector3D impulseApplied = { 0.0f,0.0f,0.0f };
	void AddForce(float xForce, float yForce, float zForce)			{ forceApplied += Vector3D(xForce, yForce, zForce);}
	void AddTorque(float xTorque, float yTorque, float zTorque)		{ torqueApplied += Vector3D(xTorque, yTorque, zTorque);}
	void AddImpulse(float xImpulse, float yImpulse, float zImpulse) { impulseApplied += Vector3D(xImpulse, yImpulse, zImpulse);}

	// linearVel and angularVel are SERIALISED authoring fields, so their stored
	// value cannot be used to detect a pending request - every body on disk
	// carries the struct default, and linearVel's default is {0,-9.81,0}.
	// Gameplay asks for a velocity change through these setters, which raise
	// velocity_dirty for PhysicsSystem to consume. Setting zero is a valid
	// request, so the FLAG - not the magnitude - is what marks one as pending.
	void SetLinearVelocity(float x, float y, float z)  { linearVel  = Vector3D(x, y, z); velocity_dirty = true; }
	void SetAngularVelocity(float x, float y, float z) { angularVel = Vector3D(x, y, z); velocity_dirty = true; }

	// Runtime only, deliberately NOT reflected: adding a reflected field would
	// change the on-disk field order for every rigid body in every scene.
	bool velocity_dirty = false;      // set by the setters above; consumed each fixed step

	bool collideWithStatic = false;		//To be used only by rigidbody Kinematic 



	Motion motion{};
	bool transform_dirty = false;     // set by gameplay when you edit Transform of kinematic/static
	bool motion_dirty = false;        // if you change Motion, flip this to trigger recreate
	uint32_t collider_seen_version = 0; // last applied Collider::version
	JPH::BodyID id = JPH::BodyID();

	RigidBodyComponent() = default;
	~RigidBodyComponent() = default;

	void SetEnabled(bool e) { enabled = e; }
	bool IsEnabled() const { return enabled; }
};