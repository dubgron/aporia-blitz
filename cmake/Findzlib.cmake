set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/zlib")

add_library(zlib STATIC
    ${ZLIB_INCLUDE_DIR}/adler32.c
    ${ZLIB_INCLUDE_DIR}/compress.c
    ${ZLIB_INCLUDE_DIR}/crc32.c
    ${ZLIB_INCLUDE_DIR}/deflate.c
    ${ZLIB_INCLUDE_DIR}/gzclose.c
    ${ZLIB_INCLUDE_DIR}/gzlib.c
    ${ZLIB_INCLUDE_DIR}/gzread.c
    ${ZLIB_INCLUDE_DIR}/gzwrite.c
    ${ZLIB_INCLUDE_DIR}/inflate.c
    ${ZLIB_INCLUDE_DIR}/infback.c
    ${ZLIB_INCLUDE_DIR}/inftrees.c
    ${ZLIB_INCLUDE_DIR}/inffast.c
    ${ZLIB_INCLUDE_DIR}/trees.c
    ${ZLIB_INCLUDE_DIR}/uncompr.c
    ${ZLIB_INCLUDE_DIR}/zutil.c)

target_include_directories(zlib PUBLIC ${ZLIB_INCLUDE_DIR})
