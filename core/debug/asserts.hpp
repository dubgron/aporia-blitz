#pragma once

#include "debug/break.hpp"
#include "debug/logger.hpp"

#if defined(APORIA_DEBUGTOOLS)

    #define APORIA_ASSERT_WITH_MESSAGE(expr, fmt, ...)  do { if (!(expr)) { APORIA_LOG(Aporia::Critical, fmt, ##__VA_ARGS__); APORIA_DEBUGBREAK(); assert(expr); } } while(0)
    #define APORIA_ASSERT(expr)                         APORIA_ASSERT_WITH_MESSAGE(expr, "Assertion '{}' failed!", #expr)
    #define APORIA_UNREACHABLE()                        APORIA_ASSERT_WITH_MESSAGE(false, "Unreachable!")

#else

    #define APORIA_ASSERT_WITH_MESSAGE
    #define APORIA_ASSERT
    #define APORIA_UNREACHABLE

#endif
