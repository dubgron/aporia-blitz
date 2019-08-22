#include <sstream>

#include <spdlog/common.h>

#include "platform.hpp"

namespace Aporia
{
    class APORIA_API LogBuffer final
    {
    public:
        LogBuffer(std::shared_ptr<spdlog::logger> logger, spdlog::level::level_enum lvl, const char* file, int line, const char* func)
            : _logger(logger), _lvl(lvl), _source(file, line, func) {}

        ~LogBuffer()
        {
            _logger->log(_source, _lvl, _buffer.str());
        }

        template<typename T>
        LogBuffer& operator<<(const T& msg)
        {
            _buffer << msg;
            return (*this);
        }

    private:
        std::shared_ptr<spdlog::logger> _logger;
        spdlog::source_loc _source;
        spdlog::level::level_enum _lvl;

        std::ostringstream _buffer;
    };
}
