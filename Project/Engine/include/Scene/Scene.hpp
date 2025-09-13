#pragma once
#include <string>

class IScene {
public:
	IScene() = default;
	IScene(const std::string& path) : scenePath(path) {}
	virtual ~IScene() = default;

	virtual void Initialize() = 0;
	virtual void Update(double dt) = 0;
	virtual void Draw() = 0;
	virtual void Exit() = 0;

protected:
	std::string scenePath{};
};