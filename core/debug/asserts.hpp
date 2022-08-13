#pragma once

#include "debug/break.hpp"
#include "debug/logger.hpp"

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_LIKELY(condition) (!!(condition))
    #define APORIA_UNLIKELY(condition) (!APORIA_LIKELY(condition))

    #define APORIA_ENSURE(condition) (APORIA_LIKELY(condition) || ( APORIA_DEBUGBREAK(), 0 ))
    #define APORIA_ENSUREF(condition, logger, fmt, ...) (APORIA_LIKELY(condition) || ( APORIA_LOG(logger, Aporia::Error, fmt, __VA_ARGS__), APORIA_DEBUGBREAK(), 0 ))

    #define APORIA_CHECK(condition) if (APORIA_UNLIKELY(condition)) { assert(condition); }
    #define APORIA_CHECKF(condition, logger, fmt, ...) if (APORIA_UNLIKELY(condition)) { APORIA_LOG(logger, Aporia::Critical, fmt, __VA_ARGS__); assert(condition); }
#else
    #define APORIA_LIKELY
    #define APORIA_UNLIKELY

    #define APORIA_ENSURE
    #define APORIA_ENSUREF

    #define APORIA_CHECK
    #define APORIA_CHECKF
#endif
