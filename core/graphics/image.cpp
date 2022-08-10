#include "image.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

namespace Aporia
{
    Image::Image(std::string_view filepath)
    {
        load(filepath);
    }

    Image::~Image()
    {
        unload();
    }

    void Image::load(std::string_view filepath)
    {
        if (_data.pixels)
        {
            unload();
        }

        _data.pixels = stbi_load(filepath.data(), &_data.width, &_data.height, &_data.channels, 4);
    }

    ImageData& Image::get_data()
    {
        return _data;
    }

    void Image::unload()
    {
        stbi_image_free(_data.pixels);
    }
}
