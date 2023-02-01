#pragma once

#include <string_view>

#include "aporia_types.hpp"

namespace Aporia
{
    struct ImageData
    {
        u8* pixels = nullptr;

        i32 width = 0;
        i32 height = 0;
        i32 channels = 0;
    };

    class Image
    {
    public:
        Image() = default;
        Image(std::string_view filepath);
        ~Image();

        void load(std::string_view filepath);
        ImageData& get_data();

    private:
        void unload();

        ImageData _data;
    };
}
