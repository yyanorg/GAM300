#pragma once

class TestScene {
public:
    static void RunAllTests();

private:
    static void TestGLFW();
    static void TestGLAD();
    static void TestGLM();
    static void TestFreeType();
    static void TestAssimp();
};