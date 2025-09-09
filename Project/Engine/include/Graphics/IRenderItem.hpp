#pragma once
class Camera;

class IRenderItem {
public:
	virtual ~IRenderItem() = default;
	virtual void Render(Camera* camera) = 0;
	virtual int GetRenderOrder() const = 0;
	virtual bool IsVisible() const { return true; }
};