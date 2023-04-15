#include "aporia_utils.hpp"

#include <fstream>

#include "aporia_debug.hpp"

namespace Aporia
{
    std::string read_file(std::string_view filepath)
    {
        APORIA_ASSERT(std::filesystem::exists(filepath));

        std::ifstream file{ filepath.data(), std::ios::in };
        APORIA_LOG(Info, "Opened '{}' successfully!", filepath);

        return std::string{ std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>() };
    }

    m4 to_mat4(const Transform2D& transform)
    {
        // @NOTE(dubgron): Precalculated following lines:
        //     result = glm::translate(glm::mat4{ 1.f }, transform.position + glm::vec3{ transform.origin, 0.f });
        //     result = glm::rotate(result, transform.rotation, glm::vec3{ 0.f, 0.f, 1.f });
        //     result = glm::scale(result, glm::vec3{ transform.scale, 1.f });
        //     result = glm::translate(result, glm::vec3{ a-transform.origin, 0.f });
        const v2 sin_scaled{ transform.scale * std::sin(transform.rotation) };
        const v2 cos_scaled{ transform.scale * std::cos(transform.rotation) };

        const f32 x_translate = transform.position.x + transform.origin.x - (transform.origin.x * cos_scaled.x - transform.origin.y * sin_scaled.y);
        const f32 y_translate = transform.position.y + transform.origin.y - (transform.origin.x * sin_scaled.x + transform.origin.y * cos_scaled.y);
        const f32 z_translate = transform.position.z;

        return m4{
            cos_scaled.x,   sin_scaled.x,   0.f,            0.f,
            -sin_scaled.y,  cos_scaled.y,   0.f,            0.f,
            0.f,            0.f,            1.f,            0.f,
            x_translate,    y_translate,    z_translate,    1.f };
    }

    Transform2D operator*(const Transform2D& transform1, const Transform2D& transform2)
    {
        return Transform2D{
            .position = transform1.position + transform2.position,
            .origin = transform1.origin + transform2.origin,
            .rotation = transform1.rotation + transform2.rotation,
            .scale = transform1.scale * transform2.scale };
    }

    f32 degrees_to_radians(f32 angle_in_degrees)
    {
        return angle_in_degrees / 180.f * M_PI;
    }

    f32 radians_to_degrees(f32 angle_in_radians)
    {
        return angle_in_radians / M_PI * 180.f;
    }

    i32 unwind_angle(i64 angle)
    {
        constexpr i32 full_angle = 360;
        return (full_angle + (angle % full_angle)) % full_angle;
    }

    Color hsv_to_rgb(i64 hue, f64 saturation, f64 value)
    {
        hue = unwind_angle(hue);
        APORIA_ASSERT(hue >= 0 && hue < 360);

        const f64 C = saturation * value;
        const f64 X = C * (1.0 - std::abs(std::fmod(hue / 60.0, 2.0) - 1.0));
        const f64 m = value - C;

        f64 Rs, Gs, Bs;
        if (hue >= 0 && hue < 60)
        {
            Rs = C;
            Gs = X;
            Bs = 0.0;
        }
        else if (hue >= 60 && hue < 120)
        {
            Rs = X;
            Gs = C;
            Bs = 0.0;
        }
        else if (hue >= 120 && hue < 180)
        {
            Rs = 0.0;
            Gs = C;
            Bs = X;
        }
        else if (hue >= 180 && hue < 240)
        {
            Rs = 0.0;
            Gs = X;
            Bs = C;
        }
        else if (hue >= 240 && hue < 300)
        {
            Rs = X;
            Gs = 0.0;
            Bs = C;
        }
        else if (hue >= 300 && hue < 360)
        {
            Rs = C;
            Gs = 0.0;
            Bs = X;
        }

        const u8 r = static_cast<u8>( (Rs + m) * 255 );
        const u8 g = static_cast<u8>( (Gs + m) * 255 );
        const u8 b = static_cast<u8>( (Bs + m) * 255 );

        return Color{ r, g, b };
    }

    static std::mt19937 random_generator{ std::random_device{}() };

    i32 random_range(i32 min, i32 max)
    {
        return std::uniform_int_distribution(min, max)(random_generator);
    }

    i64 random_range(i64 min, i64 max)
    {
        return std::uniform_int_distribution(min, max)(random_generator);
    }

    f32 random_range(f32 min, f32 max)
    {
        return std::uniform_real_distribution(min, max)(random_generator);
    }
}
