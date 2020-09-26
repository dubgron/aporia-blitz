set(GLM_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/glm")

add_library(glm INTERFACE)

target_include_directories(glm INTERFACE ${GLM_INCLUDE_DIR})
