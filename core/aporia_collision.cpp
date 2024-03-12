#include "aporia_collision.hpp"

#include "aporia_memory.hpp"
#include "aporia_rendering.hpp"
#include "aporia_utils.hpp"

namespace Aporia
{
    static inline f32 length_squared(v2 p)
    {
        return glm::dot(p, p);
    }

    static inline f32 distance_squared(v2 p0, v2 p1)
    {
        return length_squared(p1 - p0);
    }

    // @TODO(dubgron): This probably shouldn't be a separate stage. We don't need to
    // calculate all axes upfront, since if one of them is separating, then we don't
    // care about the others.
    static void gather_axis_from_shape(v2* axis, v2* points, i64 point_count)
    {
        v2 p_start = points[point_count - 1];
        for (i64 idx = 0; idx < point_count; ++idx)
        {
            v2 p_end = points[idx];
            v2 tangent = glm::normalize(p_end - p_start);
            v2 normal = v2{ tangent.y, -tangent.x };

            axis[idx] = normal;
            p_start = p_end;
        }
    }

    static void project_onto_axis(v2 axis, v2* points, i64 point_count, f32* out_lower, f32* out_upper)
    {
        *out_lower = *out_upper = glm::dot(points[0], axis);

        for (i64 idx = 1; idx < point_count; ++idx)
        {
            f32 projected = glm::dot(points[idx], axis);

            if (projected < *out_lower)
            {
                *out_lower = projected;
            }
            else if (projected > *out_upper)
            {
                *out_upper = projected;
            }
        }
    }

    static bool does_separating_axis_exist(v2* axis, i64 axis_count, v2* points0, i64 point_count0, v2* points1, i64 point_count1)
    {
        for (i64 idx = 0; idx < axis_count; ++idx)
        {
            f32 lower0, upper0;
            project_onto_axis(axis[idx], points0, point_count0, &lower0, &upper0);

            f32 lower1, upper1;
            project_onto_axis(axis[idx], points1, point_count1, &lower1, &upper1);

            if (lower0 >= upper1 || upper0 <= lower1)
            {
                return true;
            }
        }

        return false;
    }

    bool collision_point_to_aabb(v2 point, const Collider_AABB& aabb)
    {
        bool collides_on_x = point.x > aabb.base.x && point.x < aabb.base.x + aabb.width;
        bool collides_on_y = point.y > aabb.base.y && point.y < aabb.base.y + aabb.height;
        return collides_on_x && collides_on_y;
    }

    bool collision_point_to_circle(v2 point, const Collider_Circle& circle)
    {
        f32 dist_squared = distance_squared(point, circle.base);
        return dist_squared < circle.radius * circle.radius;
    }

    f32 collinearity_test(v2 p0, v2 p1, v2 p2)
    {
        // If the result is 0, the points are collinear;
        // if it is positive, the three points constitute a "left turn" or counter-clockwise orientation,
        // otherwise a "right turn" or clockwise orientation (for counter-clockwise numbered points).
        // See https://en.wikipedia.org/wiki/Graham_scan.
        return (p1.x - p0.x) * (p2.y - p0.y) - (p1.y - p0.y) * (p2.x - p0.x);
    }

    bool collision_point_to_polygon(v2 point, const Collider_Polygon& polygon)
    {
        f32 initial_orientation = collinearity_test(polygon.points[polygon.point_count - 1], polygon.points[0], point);

        for (i64 idx = 0; idx < polygon.point_count - 1; ++idx)
        {
            f32 orientation = collinearity_test(polygon.points[idx], polygon.points[idx + 1], point);

            // If orientations have different signs, it means the point is not on the same
            // side of all polygon edges, which means it is not inside the polygon.
            if (orientation * initial_orientation < 0.f)
            {
                return false;
            }
        }

        return true;
    }

    bool collision_aabb_to_aabb(const Collider_AABB& aabb_a, const Collider_AABB& aabb_b)
    {
        bool collides_on_x = (aabb_a.base.x < aabb_b.base.x + aabb_b.width) && (aabb_a.base.x + aabb_a.width > aabb_b.base.x);
        bool collides_on_y = (aabb_a.base.y < aabb_b.base.y + aabb_b.height) && (aabb_a.base.y + aabb_a.height > aabb_b.base.y);
        return collides_on_x && collides_on_y;
    }

    bool collision_aabb_to_circle(const Collider_AABB& aabb, const Collider_Circle& circle)
    {
        v2 aabb_half_extents = v2{ aabb.width, aabb.height } / 2.f;
        v2 aabb_center = aabb.base + aabb_half_extents;

        v2 from_aabb_center_to_circle = circle.base - aabb_center;
        v2 closest_on_aabb_border = aabb_center + glm::clamp(from_aabb_center_to_circle, -aabb_half_extents, aabb_half_extents);

        f32 dist_squared_from_circle_to_closest = distance_squared(circle.base, closest_on_aabb_border);
        return dist_squared_from_circle_to_closest < circle.radius * circle.radius;
    }

    bool collision_aabb_to_polygon(const Collider_AABB& aabb, const Collider_Polygon& polygon)
    {
        ScratchArena temp = scratch_begin();

        constexpr i64 aabb_axis_count = 2;

        i64 axis_count = aabb_axis_count + polygon.point_count;
        v2* axis = arena_push_uninitialized<v2>(temp.arena, axis_count);

        axis[0] = v2{ 1.f, 0.f };
        axis[1] = v2{ 0.f, 1.0f };
        gather_axis_from_shape(axis + aabb_axis_count, polygon.points, polygon.point_count);

        v2 aabb_points[] = {
            aabb.base,
            aabb.base + v2{ aabb.width, 0.f },
            aabb.base + v2{ aabb.width, aabb.height },
            aabb.base + v2{ 0.f, aabb.height }
        };

        bool collides = !does_separating_axis_exist(axis, axis_count,
            aabb_points, ARRAY_COUNT(aabb_points), polygon.points, polygon.point_count);

        scratch_end(temp);

        return collides;
    }

    bool collision_circle_to_circle(const Collider_Circle& circle_a, const Collider_Circle& circle_b)
    {
        f32 dist_squared = distance_squared(circle_a.base, circle_b.base);
        f32 sum_of_radii = circle_a.radius + circle_b.radius;
        return dist_squared < sum_of_radii * sum_of_radii;
    }

    bool collision_circle_to_polygon(const Collider_Circle& circle, const Collider_Polygon& polygon)
    {
        f32 min_dist_squared = distance_squared(circle.base, polygon.points[0]);
        i64 closest_point_idx = 0;

        for (i64 idx = 1; idx < polygon.point_count; ++idx)
        {
            f32 dist_squared = distance_squared(circle.base, polygon.points[idx]);
            if (dist_squared < min_dist_squared)
            {
                min_dist_squared = dist_squared;
                closest_point_idx = idx;
            }
        }

        v2 from_circle_to_closest = polygon.points[closest_point_idx] - circle.base;

        ScratchArena temp = scratch_begin();

        constexpr i64 circle_axis_count = 1;
        i64 axis_count = polygon.point_count + circle_axis_count;
        v2* axis = arena_push_uninitialized<v2>(temp.arena, axis_count);

        axis[0] = glm::normalize(from_circle_to_closest);
        gather_axis_from_shape(axis + circle_axis_count, polygon.points, polygon.point_count);

        bool collides = true;

        for (i64 idx = 0; idx < axis_count; ++idx)
        {
            f32 circle_projected = glm::dot(circle.base, axis[idx]);
            f32 lower0 = circle_projected - circle.radius;
            f32 upper0 = circle_projected + circle.radius;

            f32 lower1, upper1;
            project_onto_axis(axis[idx], polygon.points, polygon.point_count, &lower1, &upper1);

            if (lower0 >= upper1 || upper0 <= lower1)
            {
                collides = false;
                break;
            }
        }

        scratch_end(temp);

        return collides;
    }

    bool collision_polygon_to_polygon(const Collider_Polygon& polygon_a, const Collider_Polygon& polygon_b)
    {
        ScratchArena temp = scratch_begin();

        i64 axis_count = polygon_a.point_count + polygon_b.point_count;
        v2* axis = arena_push_uninitialized<v2>(temp.arena, axis_count);

        gather_axis_from_shape(axis, polygon_a.points, polygon_a.point_count);
        gather_axis_from_shape(axis + polygon_a.point_count, polygon_b.points, polygon_b.point_count);

        bool collides = !does_separating_axis_exist(axis, axis_count,
            polygon_a.points, polygon_a.point_count, polygon_b.points, polygon_b.point_count);

        scratch_end(temp);

        return collides;
    }

    bool collision_check(const Collider& collider_a, const Collider& collider_b)
    {
        switch (collider_a.type)
        {
            case ColliderType_AABB:
                switch (collider_b.type)
                {
                    case ColliderType_AABB:     return collision_aabb_to_aabb(collider_a.aabb, collider_b.aabb);
                    case ColliderType_Circle:   return collision_aabb_to_circle(collider_a.aabb, collider_b.circle);
                    case ColliderType_Polygon:  return collision_aabb_to_polygon(collider_a.aabb, collider_b.polygon);
                }

            case ColliderType_Circle:
                switch (collider_b.type)
                {
                    case ColliderType_AABB:     return collision_aabb_to_circle(collider_b.aabb, collider_a.circle);
                    case ColliderType_Circle:   return collision_circle_to_circle(collider_a.circle, collider_b.circle);
                    case ColliderType_Polygon:  return collision_circle_to_polygon(collider_a.circle, collider_b.polygon);
                }

            case ColliderType_Polygon:
                switch (collider_b.type)
                {
                    case ColliderType_AABB:     return collision_aabb_to_polygon(collider_b.aabb, collider_a.polygon);
                    case ColliderType_Circle:   return collision_circle_to_polygon(collider_b.circle, collider_a.polygon);
                    case ColliderType_Polygon:  return collision_polygon_to_polygon(collider_a.polygon, collider_b.polygon);
                }
        }

        return false;
    }

    void draw_collider(const Collider& collider, f32 thickness /* = 1.f */, Color color /* = Color::Magenta */)
    {
        switch (collider.type)
        {
            case ColliderType_AABB:     draw_collider_aabb(collider.aabb, thickness, color);        break;
            case ColliderType_Circle:   draw_collider_circle(collider.circle, thickness, color);    break;
            case ColliderType_Polygon:  draw_collider_polygon(collider.polygon, thickness, color);  break;
        }
    }

    void draw_collider_aabb(const Collider_AABB& aabb, f32 thickness /* = 1.f */, Color color /* = Color::Magenta */)
    {
        v2 p0 = aabb.base;
        v2 p1 = aabb.base + v2{ aabb.width, 0.f };
        v2 p2 = aabb.base + v2{ aabb.width, aabb.height };
        v2 p3 = aabb.base + v2{ 0.f, aabb.height };

        draw_line(p0, p1, thickness, color);
        draw_line(p1, p2, thickness, color);
        draw_line(p2, p3, thickness, color);
        draw_line(p3, p0, thickness, color);
    }

    void draw_collider_circle(const Collider_Circle& circle, f32 thickness /* = 1.f */, Color color /* = Color::Magenta */)
    {
        constexpr i64 segments = 32;
        constexpr f32 angle_increment = 2 * M_PI / segments;

        v2 prev_point = circle.base + v2{ circle.radius, 0.f };

        for (i64 i = 0; i < segments; ++i)
        {
            f32 angle = (i + 1) * angle_increment;
            v2 point = circle.base + circle.radius * v2{ cos(angle) , sin(angle) };

            draw_line(prev_point, point, thickness, color);

            prev_point = point;
        }
    }

    void draw_collider_polygon(const Collider_Polygon& polygon, f32 thickness /* = 1.f */, Color color /* = Color::Magenta */)
    {
        draw_line(polygon.points[polygon.point_count - 1], polygon.points[0], thickness, color);
        for (i64 idx = 0; idx < polygon.point_count - 1; ++idx)
        {
            draw_line(polygon.points[idx], polygon.points[idx + 1], thickness, color);
        }
    }

}
