set(NLOHMANN_JSON_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/nlohmann_json/include")

add_library(nlohmann_json INTERFACE)

target_include_directories(nlohmann_json INTERFACE ${NLOHMANN_JSON_INCLUDE_DIR})
