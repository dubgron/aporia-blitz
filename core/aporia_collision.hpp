#pragma once

#include "aporia_types.hpp"
#include "aporia_utils.hpp"

enum ColliderType : u8
{
    ColliderType_None,
    ColliderType_AABB,
    ColliderType_Circle,
    ColliderType_Polygon,
};

struct Collider_AABB
{
    v2 base{ 0.f };
    f32 width = 0.f;
    f32 height = 0.f;
};

struct Collider_Circle
{
    v2 base{ 0.f };
    f32 radius = 0.f;
};

struct Collider_Polygon
{
    i64 point_count = 0;
    v2* points = nullptr;
};

struct Collider
{
    ColliderType type = ColliderType_None;
    union
    {
        Collider_AABB aabb = {};
        Collider_Circle circle;
        Collider_Polygon polygon;
    };

    // @HACK(dubgron): Until Clang-17 and GCC-13, those compilers didn't properly
    // compile anonymous unions of structs without default constructors.
    Collider() {};
};

bool collision_point_to_aabb(v2 point, const Collider_AABB& aabb);
bool collision_point_to_circle(v2 point, const Collider_Circle& circle);
bool collision_point_to_polygon(v2 point, const Collider_Polygon& polygon);

bool collision_aabb_to_aabb(const Collider_AABB& aabb_a, const Collider_AABB& aabb_b);
bool collision_aabb_to_circle(const Collider_AABB& aabb, const Collider_Circle& circle);
bool collision_aabb_to_polygon(const Collider_AABB& aabb, const Collider_Polygon& polygon);
bool collision_circle_to_circle(const Collider_Circle& circle_a, const Collider_Circle& circle_b);
bool collision_circle_to_polygon(const Collider_Circle& circle, const Collider_Polygon& polygon);
bool collision_polygon_to_polygon(const Collider_Polygon& polygon_a, const Collider_Polygon& polygon_b);

bool collision_check(const Collider& collider_a, const Collider& collider_b);

// Debug Visualization
void draw_collider(const Collider& collider, f32 thickness = 1.f, Color color = Color::Magenta);
void draw_collider_aabb(const Collider_AABB& aabb, f32 thickness = 1.f, Color color = Color::Magenta);
void draw_collider_circle(const Collider_Circle& circle, f32 thickness = 1.f, Color color = Color::Magenta);
void draw_collider_polygon(const Collider_Polygon& polygon, f32 thickness = 1.f, Color color = Color::Magenta);

String collider_type_to_string(ColliderType type);
