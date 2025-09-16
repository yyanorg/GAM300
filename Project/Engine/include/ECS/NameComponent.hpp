#pragma once
#include <string>
struct NameComponent
{
	std::string name;

	NameComponent() = default;
	NameComponent(const std::string _name) : name(_name) {}
};