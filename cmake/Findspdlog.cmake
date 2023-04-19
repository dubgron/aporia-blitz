set(SPDLOG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include")
set(SPDLOG_SOURCE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/spdlog/src")

option(SPDLOG_USE_UNITY_BUILD "Compile spdlog using unity build" ON)

add_library(spdlog STATIC
    ${SPDLOG_SOURCE_DIR}/spdlog.cpp
    ${SPDLOG_SOURCE_DIR}/stdout_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/color_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/file_sinks.cpp
    ${SPDLOG_SOURCE_DIR}/async.cpp
    ${SPDLOG_SOURCE_DIR}/cfg.cpp
    ${SPDLOG_SOURCE_DIR}/fmt.cpp)

target_compile_features(spdlog PRIVATE cxx_std_11)
target_include_directories(spdlog PUBLIC ${SPDLOG_INCLUDE_DIR})
target_compile_definitions(spdlog PUBLIC
    SPDLOG_COMPILED_LIB
    SPDLOG_NO_EXCEPTIONS)

if (SPDLOG_USE_UNITY_BUILD)
    # UNITY_BUILD_BATCH_SIZE of 0 means combining all sources for the target into a single unity file
    set_target_properties(spdlog PROPERTIES
        UNITY_BUILD             ON
        UNITY_BUILD_MODE        BATCH
        UNITY_BUILD_BATCH_SIZE  0)
endif()
