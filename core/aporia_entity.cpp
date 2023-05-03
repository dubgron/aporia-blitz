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

    void entity_ajust_size_to_texture(Entity& entity)
    {
        if (entity.texture)
        {
            entity.width = get_subtexture_width(*entity.texture);
            entity.height = get_subtexture_height(*entity.texture);
        }
    }
}
