#include "aporia_assets.hpp"

#include <stb_sprintf.h>

#include "aporia_config.hpp"
#include "aporia_debug.hpp"
#include "platform/aporia_os.hpp"

namespace Aporia
{
    static MemoryArena assets_arena;
    static Mutex assets_mutex;

    static constexpr u64 MAX_ASSETS = 100;

    static Asset assets[MAX_ASSETS];
    static i64 assets_count = 0;

    static Asset* free_list = nullptr;

    static String asset_type_to_string(AssetType type)
    {
        switch (type)
        {
            case AssetType::Invalid:        return "Invalid";
            case AssetType::Config:         return "Config";
            case AssetType::Shader:         return "Shader";
            case AssetType::Texture:        return "Texture";
        }
        APORIA_UNREACHABLE();
        return String{};
    }

    static String asset_status_to_string(AssetStatus status)
    {
        switch (status)
        {
            case AssetStatus::NotLoaded:    return "NotLoaded";
            case AssetStatus::Loaded:       return "Loaded";
            case AssetStatus::Unloaded:     return "Unloaded";
            case AssetStatus::NeedsReload:  return "NeedsReload";
        }
        APORIA_UNREACHABLE();
        return String{};
    }

    void assets_init()
    {
        // @TODO(dubgron): The size of this arena should be more planned out.
        assets_arena.alloc(KILOBYTES(100));

        assets_mutex = mutex_create();

        free_list = assets;
        for (i64 idx = 0; idx < MAX_ASSETS - 1; ++idx)
        {
            assets[idx].next = &assets[idx + 1];
        }
    }

    void assets_deinit()
    {
        mutex_destroy(&assets_mutex);
    }

    void assets_reload_if_dirty()
    {
        if (!mutex_try_lock(&assets_mutex))
        {
            return;
        }

        for (i64 idx = 0; idx < assets_count; ++idx)
        {
            Asset* asset = &assets[idx];
            if (asset->status == AssetStatus::NeedsReload)
            {
                switch (asset->type)
                {
                    case AssetType::Config:     reload_config_asset(asset);     break;
                    case AssetType::Shader:     reload_shader_asset(asset);     break;
                    case AssetType::Texture:    reload_texture_asset(asset);    break;
                }
            }
        }

        mutex_unlock(&assets_mutex);
    }

    Asset* register_asset(String source_file, AssetType type)
    {
        if (type == AssetType::Invalid)
        {
            APORIA_LOG(Warning, "Can't register an asset '%' with an invalid asset type!", source_file);
            return nullptr;
        }

        if (!free_list)
        {
            APORIA_LOG(Warning, "Failed to register an asset '%' of type %! The asset list is full!", source_file, asset_type_to_string(type));
            return nullptr;
        }

        static i64 next_id = 0;

        Asset result;
        result.id = next_id;
        result.source_file = push_string(&assets_arena, source_file);
        result.type = type;
        result.status = AssetStatus::NotLoaded;

        Asset* new_created = free_list;
        free_list = free_list->next;

        *new_created = result;

        assets_count += 1;
        next_id += 1;

        APORIA_LOG(Info, "Registered asset '%' of type %", source_file, asset_type_to_string(type));

        return new_created;
    }

    bool unregister_asset(i64 id)
    {
        Asset* asset = get_asset(id);
        if (!asset)
        {
            APORIA_LOG(Warning, "Failed to unregister an asset with ID %! No asset with this ID exists!", id);
            return false;
        }

        if (assets_count > 1)
        {
            *asset = assets[assets_count - 1];
            asset = &assets[assets_count - 1];
        }

        *asset = Asset{};
        asset->next = free_list;
        free_list = asset;

        assets_count -= 1;

        return true;
    }

    Asset* get_asset(i64 id)
    {
        Asset* result = nullptr;
        if (id == Asset::INVALID_ID)
        {
            return result;
        }

        for (i64 idx = 0; idx < assets_count; ++idx)
        {
            if (assets[idx].id == id)
            {
                result = &assets[idx];
                break;
            }
        }
        return result;
    }

    Asset* get_asset_by_source_file(String source_file)
    {
        Asset* result = nullptr;
        for (i64 idx = 0; idx < assets_count; ++idx)
        {
            if (assets[idx].source_file == source_file)
            {
                result = &assets[idx];
                break;
            }
        }
        return result;
    }

    void asset_change_status(Asset* asset, AssetStatus status)
    {
        mutex_lock(&assets_mutex);
        asset->status = status;
        mutex_unlock(&assets_mutex);
    }

#if defined(APORIA_DEBUGTOOLS)

#define SELECTABLE_ASSET_TYPE(asset_type) if (ImGui::Selectable(*asset_type_to_string(asset_type), type == (u8)asset_type)) type = (u8)asset_type

    void debug_assets()
    {
        ImGui::Begin("Debug | Assets");

        static char buff[256] = { '\0' };
        static u8 type = 0;
        static bool selected = false;

        ImGui::InputText("Asset Source File", buff, ARRAY_COUNT(buff));

        SELECTABLE_ASSET_TYPE(AssetType::Invalid);
        SELECTABLE_ASSET_TYPE(AssetType::Config);
        SELECTABLE_ASSET_TYPE(AssetType::Shader);
        SELECTABLE_ASSET_TYPE(AssetType::Texture);

        if (ImGui::Button("Regiser Asset"))
        {
            String s = push_string(&persistent_arena, buff);
            register_asset(s, (AssetType)type);
        }
        ImGui::Separator();

        if (assets_count > 0)
        {
            static i32 selected_id = Asset::INVALID_ID;

            ImGui::InputInt("Asset ID", &selected_id);
            if (ImGui::Button("Unregiser Asset"))
            {
                unregister_asset(selected_id);
                selected_id = Asset::INVALID_ID;
            }
            ImGui::SameLine();
            if (ImGui::Button("Mark Asset As Dirty"))
            {
                get_asset(selected_id)->status = AssetStatus::NeedsReload;
            }
            ImGui::Separator();


            if (ImGui::BeginTable("Registered Assets", 4, ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("ID");
                ImGui::TableSetupColumn("Source File");
                ImGui::TableSetupColumn("Type");
                ImGui::TableSetupColumn("Status");
                ImGui::TableHeadersRow();
                for (i64 idx = 0; idx < assets_count; ++idx)
                {
                    Asset* asset = &assets[idx];

                    ImGui::TableNextRow();

                    ImGui::TableNextColumn();

                    char label[8];
                    stbsp_sprintf(label, "%d", asset->id);
                    if (ImGui::Selectable(label, asset->id == selected_id, ImGuiSelectableFlags_SpanAllColumns))
                    {
                        selected_id = asset->id;
                    }

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", *asset->source_file);

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", *asset_type_to_string(asset->type));

                    ImGui::TableNextColumn();
                    ImGui::Text("%s", *asset_status_to_string(asset->status));
                }

                ImGui::EndTable();
            }
        }

        ImGui::End();
    }
#endif
}
