#include <fstream>
#include <string>

namespace Aporia::Utils
{
    inline std::string read_file(const std::string& path)
    {
        std::ifstream file(path, std::ios::binary);
        return std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    }
}