#pragma once

#include <optional>
#include <assert.h>
#include <string>

#include "Component.hpp"
#include "ComponentArray.hpp"

namespace {
	template <typename T>
	std::string GetReadableTypeName() {
		const char* rawName = typeid(T).name();
		std::string typeName = rawName;

		if (typeName.find("struct ") == 0) {
			typeName = typeName.substr(7);
		}
		else if (typeName.find("class ") == 0) {
			typeName = typeName.substr(6);
		}
		return typeName;
	}
}

class ComponentManager {
public:
	template <typename T>
	void RegisterComponent() {
		std::string typeName = GetReadableTypeName<T>();
		assert(components.find(typeName) == components.end() && "Registering component type more than once.");
		components[typeName] = nextComponentID;
		componentArrays[typeName] = std::make_shared<ComponentArray<T>>();
		++nextComponentID;
	}

	template <typename T>
	ComponentID GetComponentID() {
		std::string typeName = GetReadableTypeName<T>();
		assert(components.find(typeName) != components.end() && "Component not registered before use.");
		return components[typeName];
	}

	template <typename T>
	void AddComponent(Entity entity, T component) {
		GetComponentArray<T>()->InsertComponent(entity, component);
	}

	template <typename T>
	void RemoveComponent(Entity entity) {
		GetComponentArray<T>()->RemoveComponent(entity);
	}

	template <typename T>
	T& GetComponent(Entity entity) {
		return GetComponentArray<T>()->GetComponent(entity);
	}

	template <typename T>
	std::optional<std::reference_wrapper<T>> TryGetComponent(Entity entity) {
		return GetComponentArray<T>()->TryGetComponent(entity);
	}

	void EntityDestroyed(Entity entity) {
		for (auto const& pair : componentArrays) {
			auto const& componentArray = pair.second;
			componentArray->EntityDestroyed(entity);
		}
	}

	void AllEntitiesDestroyed() {
		for (auto const& pair : componentArrays) {
			auto const& componentArray = pair.second;
			componentArray->AllEntitiesDestroyed();
		}
	}

private:
	std::unordered_map<std::string, ComponentID> components{}; // Map from component type name to component ID.
	std::unordered_map<std::string, std::shared_ptr<IComponentArray>> componentArrays{}; // Map from component type name to component array.
	ComponentID nextComponentID{}; // The next available component ID to assign.

	template<typename T>
	std::shared_ptr<ComponentArray<T>> GetComponentArray() {
		std::string typeName = GetReadableTypeName<T>();

		assert(components.find(typeName) != components.end() && "Component not registered before use.");

		return std::static_pointer_cast<ComponentArray<T>>(componentArrays[typeName]);
	}
};