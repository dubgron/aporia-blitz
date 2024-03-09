#include "aporia_entity.hpp"

namespace Aporia
{
    bool entity_flag_is_set(const Entity& entity, EntityFlag flag)
    {
        return entity.flags & flag;
    }

    void entity_flag_set(Entity* entity, EntityFlag flag)
    {
        entity->flags |= flag;
    }

    void entity_flag_unset(Entity* entity, EntityFlag flag)
    {
        entity->flags &= ~flag;
    }

    void entity_ajust_size_to_texture(Entity* entity)
    {
        get_subtexture_size(entity->texture, &entity->width, &entity->height);
    }
}
