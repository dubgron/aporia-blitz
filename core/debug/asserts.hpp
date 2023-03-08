#pragma once

#include "debug/break.hpp"
#include "debug/logger.hpp"

#if defined(APORIA_DEBUGTOOLS)
    #define APORIA_ASSERT(cond)                                 if (!(cond)) { APORIA_DEBUGBREAK(); assert(cond); }
    #define APORIA_ASSERT_WITH_MESSAGE(cond, fmt, ...)          if (!(cond)) { APORIA_LOG(Aporia::Critical, fmt, ##__VA_ARGS__); APORIA_DEBUGBREAK(); assert(cond); }
    #define APORIA_ASSERT_NO_ENTRY()                            APORIA_ASSERT(false)
#else
    #define APORIA_ASSERT
    #define APORIA_ASSERT_WITH_MESSAGE
    #define APORIA_ASSERT_NO_ENTRY
#endif

#define APORIA_VALIDATE_OR_EXECUTE(cond, code, fmt, ...)        if (!(cond)) { APORIA_LOG(Aporia::Error, fmt, ##__VA_ARGS__); code; }

#define APORIA_VALIDATE_OR_RETURN(cond, fmt, ...)               APORIA_VALIDATE_OR_EXECUTE(cond, return, fmt, ##__VA_ARGS__)
#define APORIA_VALIDATE_OR_RETURN_VALUE(cond, value, fmt, ...)  APORIA_VALIDATE_OR_EXECUTE(cond, return value, fmt, ##__VA_ARGS__)

#define APORIA_VALIDATE_OR_CONTINUE(cond, fmt, ...)             APORIA_VALIDATE_OR_EXECUTE(cond, continue, fmt, ##__VA_ARGS__)
#define APORIA_VALIDATE_OR_BREAK(cond, fmt, ...)                APORIA_VALIDATE_OR_EXECUTE(cond, break, fmt, ##__VA_ARGS__)
