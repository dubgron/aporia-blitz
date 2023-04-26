#include "aporia_os.hpp"

#if defined(APORIA_WINDOWS)
    #include "aporia_win32.cpp"
#elif defined(APORIA_UNIX)
    #include "aporia_unix.cpp"
#else
    #error OS not supported!
#endif
