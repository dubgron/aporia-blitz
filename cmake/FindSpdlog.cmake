set(SPDLOG_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include")

add_library(spdlog INTERFACE)

target_include_directories(spdlog INTERFACE ${SPDLOG_INCLUDE_DIR})
