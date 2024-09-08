set(ZLIB_INCLUDE_DIR "${PROJECT_SOURCE_DIR}/thirdparty/zlib")

include(CheckTypeSize)
include(CheckFunctionExists)
include(CheckIncludeFile)

# Check to see if we have large file support
check_type_size(off64_t OFF64_T)
if(HAVE_OFF64_T)
   add_definitions(-D_LARGEFILE64_SOURCE=1)
endif()

# Check for fseeko
check_function_exists(fseeko HAVE_FSEEKO)
if(NOT HAVE_FSEEKO)
    add_definitions(-DNO_FSEEKO)
endif()

# Check for unistd.h
check_include_file(unistd.h Z_HAVE_UNISTD_H)

if(MSVC)
    add_definitions(-D_CRT_SECURE_NO_DEPRECATE)
    add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE)
endif()

configure_file( ${ZLIB_INCLUDE_DIR}/zlib.pc.cmakein ${ZLIB_INCLUDE_DIR}/zlib.pc @ONLY)
configure_file(	${ZLIB_INCLUDE_DIR}/zconf.h.cmakein ${ZLIB_INCLUDE_DIR}/zconf.h @ONLY)

add_library(zlib STATIC
    ${ZLIB_INCLUDE_DIR}/zconf.h
    ${ZLIB_INCLUDE_DIR}/zlib.h
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
