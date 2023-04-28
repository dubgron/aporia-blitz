#include "aporia_entity.hpp"

namespace Aporia
{
    bool is_flag_set(const Entity& entity, EntityFlag flag)
    {
        return entity.flags & flag;
    }

    void set_flag(Entity& entity, EntityFlag flag)
    {
        entity.flags |= flag;
    }

    void unset_flag(Entity& entity, EntityFlag flag)
    {
        entity.flags &= ~flag;
    }
}
