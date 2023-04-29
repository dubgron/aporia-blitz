#pragma once

#include <memory>
#include <string_view>

#include <spdlog/logger.h>

#include "aporia_strings.hpp"
#include "aporia_types.hpp"

#if defined(APORIA_DEBUGTOOLS)

    #define APORIA_LOG(lvl, fmt, ...)                   Aporia::log(__FILE__, __LINE__, __func__, lvl, fmt, ##__VA_ARGS__)

    #define APORIA_ASSERT_WITH_MESSAGE(expr, fmt, ...)  do { if (!(expr)) { APORIA_LOG(Aporia::Critical, fmt, ##__VA_ARGS__); APORIA_BREAKPOINT(); assert(expr); } } while(0)
    #define APORIA_ASSERT(expr)                         APORIA_ASSERT_WITH_MESSAGE(expr, "Assertion '{}' failed!", #expr)
    #define APORIA_UNREACHABLE()                        APORIA_ASSERT_WITH_MESSAGE(false, "Unreachable!")

    #if defined(__has_builtin) && !defined(__ibmxl__)
        #if __has_builtin(__builtin_debugtrap)
            #define APORIA_BREAKPOINT() __builtin_debugtrap()
        #elif __has_builtin(__debugbreak)
            #define APORIA_BREAKPOINT() __debugbreak()
        #endif
    #endif

    #if !defined(APORIA_BREAKPOINT)
        #if defined(_MSC_VER) || defined(__INTEL_COMPILER)
            #define APORIA_BREAKPOINT() __debugbreak()
        #elif defined(__ARMCC_VERSION)
            #define APORIA_BREAKPOINT() __breakpoint(42)
        #elif defined(__ibmxl__) || defined(__xlC__)
            #include <builtins.h>
            #define APORIA_BREAKPOINT() __trap(42)
        #elif defined(__DMC__) && defined(_M_IX86)
            static inline void APORIA_BREAKPOINT(void) { __asm int 3h; }
        #elif defined(__i386__) || defined(__x86_64__)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__("int3"); }
        #elif defined(__thumb__)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__(".inst 0xde01"); }
        #elif defined(__aarch64__)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__(".inst 0xd4200000"); }
        #elif defined(__arm__)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__(".inst 0xe7f001f0"); }
        #elif defined (__alpha__) && !defined(__osf__)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__("bpt"); }
        #elif defined(_54_)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__("ESTOP"); }
        #elif defined(_55_)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__(";\n .if (.MNEMONIC)\n ESTOP_1\n .else\n ESTOP_1()\n .endif\n NOP"); }
        #elif defined(_64P_)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__("SWBP 0"); }
        #elif defined(_6x_)
            static inline void APORIA_BREAKPOINT(void) { __asm__ __volatile__("NOP\n .word 0x10000000"); }
        #elif defined(__STDC_HOSTED__) && (__STDC_HOSTED__ == 0) && defined(__GNUC__)
            #define APORIA_BREAKPOINT() __builtin_trap()
        #else
            #include <signal.h>
            #if defined(SIGTRAP)
                #define APORIA_BREAKPOINT() raise(SIGTRAP)
            #else
                #define APORIA_BREAKPOINT() raise(SIGABRT)
            #endif
        #endif
    #endif

#else

    #define APORIA_LOG(...)
    #define APORIA_ASSERT_WITH_MESSAGE(...)
    #define APORIA_ASSERT(...)
    #define APORIA_UNREACHABLE(...)
    #define APORIA_BREAKPOINT(...)

#endif

namespace Aporia
{
    enum LogLevel
    {
        Verbose     = spdlog::level::trace,
        Debug       = spdlog::level::debug,
        Info        = spdlog::level::info,
        Warning     = spdlog::level::warn,
        Error       = spdlog::level::err,
        Critical    = spdlog::level::critical,
        Off         = spdlog::level::off
    };

    void logging_init(const std::string& name);

    template<typename... Args>
    static void log(const char* file, i32 line, const char* func, LogLevel lvl, std::string_view fmt, Args&&... args)
    {
        extern std::shared_ptr<spdlog::logger> logger;
        logger->log(spdlog::source_loc{ file, line, func }, static_cast<spdlog::level::level_enum>(lvl), fmt, std::forward<Args>(args)...);
    }

    void imgui_init();
    void imgui_deinit();

    void imgui_frame_begin();
    void imgui_frame_end();
}