#pragma once

#include <cstdint>
#include <string_view>

namespace Aporia
{
    struct ImageData
    {
        uint8_t* pixels = nullptr;

        int32_t width = 0;
        int32_t height = 0;
        int32_t channels = 0;
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
