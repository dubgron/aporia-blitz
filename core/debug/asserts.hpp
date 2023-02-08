#pragma once

#include "debug/break.hpp"
#include "debug/logger.hpp"

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_ENSURE(condition)        (!!(condition) || (APORIA_DEBUGBREAK(), 0))
    #define APORIA_ENSUREF(condition, ...)  (!!(condition) || (APORIA_LOG(Aporia::Error, __VA_ARGS__), APORIA_DEBUGBREAK(), 0))

    #define APORIA_CHECK(condition)         if (!(condition)) { assert(condition); }
    #define APORIA_CHECKF(condition, ...)   if (!(condition)) { APORIA_LOG(Aporia::Critical, __VA_ARGS__); assert(condition); }
#else
    #define APORIA_ENSURE
    #define APORIA_ENSUREF

    #define APORIA_CHECK
    #define APORIA_CHECKF
#endif

#define APORIA_VALIDATE_OR_EXECUTE(cond, code, ...)         if (!(cond)) { APORIA_LOG(Aporia::Error, __VA_ARGS__); code; }

#define APORIA_VALIDATE_OR_RETURN(cond, ...)                APORIA_VALIDATE_OR_EXECUTE(cond, return, __VA_ARGS__)
#define APORIA_VALIDATE_OR_RETURN_VALUE(cond, value, ...)   APORIA_VALIDATE_OR_EXECUTE(cond, return value, __VA_ARGS__)

#define APORIA_VALIDATE_OR_CONTINUE(cond, ...)              APORIA_VALIDATE_OR_EXECUTE(cond, continue, __VA_ARGS__)
#define APORIA_VALIDATE_OR_BREAK(cond, ...)                 APORIA_VALIDATE_OR_EXECUTE(cond, break, __VA_ARGS__)
