#pragma once

#include "aporia_string.hpp"
#include "aporia_types.hpp"

namespace Aporia
{
    enum class AssetType : u8
    {
        Invalid = 0,
        Config,
        Shader,
        Texture,
    };

    enum class AssetStatus : u8
    {
        NotLoaded,
        Loaded,
        Unloaded,
        NeedsReload,
    };

    struct Asset
    {
        Asset* next = nullptr;

        i64 id = INVALID_ID;
        String source_file;
        AssetType type = AssetType::Invalid;
        AssetStatus status = AssetStatus::NotLoaded;

        static constexpr i64 INVALID_ID = -1;
    };

    void assets_init();
    void assets_deinit();

    void assets_reload_if_dirty();

    Asset* register_asset(String source_file, AssetType type);
    bool unregister_asset(i64 id);

    Asset* get_asset(i64 id);
    Asset* get_asset_by_source_file(String source_file);

    void asset_change_status(Asset* asset, AssetStatus status);

#if defined(APORIA_DEBUGTOOLS)
    void debug_assets();
#endif
}
