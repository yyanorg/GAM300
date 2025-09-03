#pragma once

#include <unordered_map>

#include "Signature.hpp"
#include "System.hpp"
#include <memory>

class SystemManager {
public:
	template <typename T>
	std::shared_ptr<T> RegisterSystem() {
		std::string typeName = typeid(T).name();

		assert(systems.find(typeName) == systems.end() && "Registering system more than once.");

		// Create a shared pointer for the system and return it.
		auto system = std::make_shared<T>();
		systems[typeName] = system;
		return system;
	}

	template <typename T>
	void SetSignature(Signature signature) {
		std::string typeName = typeid(T).name();

		assert(systems.find(typeName) != systems.end() && "System used before registered.");

		signatures[typeName] = signature;
	}

	void EntityDestroyed(Entity entity) {
		for (auto const& pair : systems) {
			auto const& system = pair.second;
			system->entities.erase(entity);
		}
	}

	void AllEntitiesDestroyed() {
		for (auto const& pair : systems) {
			auto const& system = pair.second;
			system->entities.clear();
		}
	}

	void OnEntitySignatureChanged(Entity entity, Signature entitySignature) {
		for (const auto& pair : systems) {
			const auto& typeName = pair.first;
			const auto& system = pair.second;
			const auto& systemSignature = signatures[typeName];

			// If the entity's signature matches the system's signature, add it to the set.
			if ((entitySignature & systemSignature) == systemSignature) {
				system->entities.insert(entity);
			} else {
				system->entities.erase(entity);
			}
		}
	}

private:
	std::unordered_map<std::string, Signature> signatures{}; // Map from system type name to its signature.
	std::unordered_map<std::string, std::shared_ptr<System>> systems{}; // Map from system type name to a system instance.
};