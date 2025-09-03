#pragma once

#include "Entity.hpp"
#include <set>

class System {
public:
	std::set<Entity> entities; // Set of entities that are part of this system.
};