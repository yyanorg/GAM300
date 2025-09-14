#pragma once

#include <unordered_map>
#include "ECSManager.hpp"
#include "../Engine.h"  // For ENGINE_API macro

/**
 * \class ECSRegistry
 * \brief Singleton registry that manages all active ECSManager instances (worlds/scenes) in the application.
 *
 * The ECSRegistry stores and organizes multiple ECSManager instances, each representing an independent world or scene.
 * This design improves engine flexibility by enabling better scene management, isolated simulations, and potential
 * parallel execution of different worlds.
 */
class ENGINE_API ECSRegistry {
public:
	// Delete copy constructor and assignment operator to enforce singleton pattern.
	ECSRegistry(const ECSRegistry&) = delete;
	ECSRegistry& operator=(const ECSRegistry&) = delete;

	static ECSRegistry& GetInstance();

	ECSManager& CreateECSManager(const std::string& name);
	void DestroyECSManager(const std::string& name);
	ECSManager& GetECSManager(const std::string& name);

	void SetActiveECSManager(const std::string& name);
	ECSManager& GetActiveECSManager();


private:
	ECSRegistry() {};
	~ECSRegistry() {};

	std::unordered_map<std::string, std::shared_ptr<ECSManager>> ecsManagers;
	std::shared_ptr<ECSManager> activeECSManager;
};