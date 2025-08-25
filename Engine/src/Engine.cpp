#include "Engine.h"
#include "TestScene.h"

void Engine::Initialize() {
    TestScene::Initialize();
}

void Engine::Update() {
    TestScene::Update();
    TestScene::Render();
}

void Engine::Shutdown() {
    TestScene::Shutdown();
}

bool Engine::IsRunning() {
    return TestScene::IsRunning();
}
