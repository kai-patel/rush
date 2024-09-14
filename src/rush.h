#pragma once

#ifdef _WIN32
#define RUSH_EXPORT __declspec(dllexport)
#else
#define RUSH_EXPORT
#endif

RUSH_EXPORT void rush();
