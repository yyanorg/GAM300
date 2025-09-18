#pragma once

#include <rapidjson/document.h>

#include "pch.h"

//Sticking with this for now unless stated need to move to a consolidated API.hpp file
#ifdef _WIN32
#ifdef ENGINE_EXPORTS
#define ENGINE_API __declspec(dllexport)
#else
#define ENGINE_API __declspec(dllimport)
#endif
#else
// Linux/GCC
#ifdef ENGINE_EXPORTS
#define ENGINE_API __attribute__((visibility("default")))
#else
#define ENGINE_API
#endif
#endif

// The compiler ensures that the starting address of the variable is divisible by X.
// 
// Example usage: __FLX_ALIGNAS(16) float myArray[4];
// Each float is 4 bytes, so the array is 16 bytes.
// 
// Proper alignment can improve memory access performance, particularly when dealing with
// vectorized operations, SIMD (Single Instruction, Multiple Data) instructions, or GPU operations.
// 
// Misaligned memory access can result in performance penalties because the CPU or GPU
// may need to perform additional work to handle unaligned access.
#define ENGINE_ALIGNAS(X) alignas(X)

#pragma region Macros


#pragma endregion
