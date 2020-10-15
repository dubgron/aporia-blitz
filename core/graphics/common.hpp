#pragma once

namespace Aporia
{
#   if defined(APORIA_EMSCRIPTEN)
        using texture_id = float;
#   else
#       include <cstdint>
        using texture_id = uint32_t;
#   endif
}
