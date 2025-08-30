#include "pch.h"
#include <iostream>
#include <assert.h>
#include "ECSRegistry.hpp"

ECSRegistry& ECSRegistry::GetInstance() {
	static ECSRegistry instance;
	return instance;
}

ECSManager& ECSRegistry::CreateECSManager(const std::string& name) {
	assert(ecsManagers.find(name) == ecsManagers.end() && "ECSManager with the given name already exists.");

	ecsManagers[name] = std::make_shared<ECSManager>();

	// If there's no active ECSManager, set the newly created one as active.
	if (activeECSManager.get() == nullptr) {
		activeECSManager = ecsManagers[name];
	}

	std::cout << "[ECSRegistry] Created ECSManager '" << name << "'." << std::endl;
	return *ecsManagers[name];
}

ECSManager& ECSRegistry::GetECSManager(const std::string& name) {
	assert(ecsManagers.find(name) != ecsManagers.end() && "ECSManager with the given name does not exist.");
	return *ecsManagers[name];
}

void ECSRegistry::DestroyECSManager(const std::string& name) {
	assert(ecsManagers.find(name) != ecsManagers.end() && "ECSManager with the given name does not exist.");
	auto ecsManagerToDelete = ecsManagers[name];
	assert(ecsManagerToDelete != activeECSManager && "Destroying active ECS manager.");
	
	ecsManagers.erase(name);
}

void ECSRegistry::SetActiveECSManager(const std::string& name) {
	assert(ecsManagers.find(name) != ecsManagers.end() && "ECSManager with the given name does not exist.");
	activeECSManager = ecsManagers[name];
}

ECSManager& ECSRegistry::GetActiveECSManager() {
	assert(activeECSManager.get() != nullptr && "No active ECSManager set.");
	return *activeECSManager;
}
