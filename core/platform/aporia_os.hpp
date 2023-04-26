#pragma once

#include "aporia_strings.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    void* load_library(String library_name);
    bool close_library(void* library_handle);

    void* load_symbol(void* library_handle, String symbol_name);

    String get_library_error();
}
