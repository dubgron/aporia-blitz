set(MAGIC_ENUM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/magic_enum/include")

add_library(magic_enum INTERFACE)

target_include_directories(magic_enum INTERFACE ${MAGIC_ENUM_INCLUDE_DIR})
