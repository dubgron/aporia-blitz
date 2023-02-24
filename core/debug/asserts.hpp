#pragma once

#include "debug/break.hpp"
#include "debug/logger.hpp"

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_ENSURE(condition)                (!!(condition) || (APORIA_DEBUGBREAK(), 0))
    #define APORIA_ENSUREF(condition, fmt, ...)     (!!(condition) || (APORIA_LOG(Aporia::Error, fmt, ##__VA_ARGS__), APORIA_DEBUGBREAK(), 0))

    #define APORIA_CHECK(condition)                 if (!(condition)) { assert(condition); }
    #define APORIA_CHECKF(condition, fmt, ...)      if (!(condition)) { APORIA_LOG(Aporia::Critical, fmt, ##__VA_ARGS__); assert(condition); }
#else
    #define APORIA_ENSURE
    #define APORIA_ENSUREF

    #define APORIA_CHECK
    #define APORIA_CHECKF
#endif

#define APORIA_VALIDATE_OR_EXECUTE(cond, code, fmt, ...)         if (!(cond)) { APORIA_LOG(Aporia::Error, fmt, ##__VA_ARGS__); code; }

#define APORIA_VALIDATE_OR_RETURN(cond, fmt, ...)                APORIA_VALIDATE_OR_EXECUTE(cond, return, fmt, ##__VA_ARGS__)
#define APORIA_VALIDATE_OR_RETURN_VALUE(cond, value, fmt, ...)   APORIA_VALIDATE_OR_EXECUTE(cond, return value, fmt, ##__VA_ARGS__)

#define APORIA_VALIDATE_OR_CONTINUE(cond, fmt, ...)              APORIA_VALIDATE_OR_EXECUTE(cond, continue, fmt, ##__VA_ARGS__)
#define APORIA_VALIDATE_OR_BREAK(cond, fmt, ...)                 APORIA_VALIDATE_OR_EXECUTE(cond, break, fmt, ##__VA_ARGS__)
