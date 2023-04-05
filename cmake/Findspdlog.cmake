set(SPDLOG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include")
set(SPDLOG_SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/spdlog/src")

add_library(spdlog STATIC
    ${SPDLOG_SOURCE_DIR}/spdlog.cpp
    ${SPDLOG_SOURCE_DIR}/stdout_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/color_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/file_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/async.cpp
    ${SPDLOG_SOURCE_DIR}/cfg.cpp
    ${SPDLOG_SOURCE_DIR}/fmt.cpp)

target_include_directories(spdlog PUBLIC ${SPDLOG_INCLUDE_DIR})
target_compile_definitions(spdlog PUBLIC
    SPDLOG_COMPILED_LIB
    SPDLOG_NO_EXCEPTIONS)
