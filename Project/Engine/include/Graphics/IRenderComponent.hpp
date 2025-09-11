#pragma once

class IRenderComponent {
public:
	bool isVisible = true;
	int renderOrder = 100;

	IRenderComponent() = default;
	virtual ~IRenderComponent() = default;

};