#include "EditorSelectionPage.h"
#include "../../../../GlobalSettings.h"
#include "PackageManager/PackageManager.h"
#include "DebugPackManager/DebugPackManager.h"
#include "DebugPackExtractAll/ExtractAllPageDebug.h"
#include "../GUI.h"
//#include "BundleManager/BundleManager.h"
#include"ExtractAllPage.h"

void EditorPage::render(GUI& gui)
{
    ImGui::Begin("Editor Selection");

    // Use a vertical list layout for all options
    struct ListEntry {
        const char* child_id;
        ImVec4 bg_color;
        ImVec4 border_color;
        ImVec4 button_color;
        const char* desc1;
        const char* desc2;
        const char* button_text;
        bool enabled;
        std::function<void()> on_click;
    };

    std::vector<ListEntry> entries = {
        {
            "PackageManagerButton",
            ImVec4(0.6f, 0.6f, 1.0f, 1.0f),
            ImVec4(0.0f, 0.0f, 1.0f, 1.0f),
            ImVec4(0.2f, 0.2f, 0.8f, 1.0f),
            "Used to browse and modify",
            ".pkg files",
            "Open",
            true,
            [&]() { gui.CurrentPage = std::make_unique<PackageManager>(); }
        },
        {
            "DebugManagerButton",
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
            ImVec4(0.25f, 0.25f, 0.25f, 1.0f),
            "Used to browse and modify the",
            "debug_pack.bin",
            "Open",
            true,
            [&]() { gui.CurrentPage = std::make_unique<DebugPackManager>(); }
        },
        {
            "DebugExtractButton",
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
            ImVec4(0.25f, 0.25f, 0.25f, 1.0f),
            "Used to Extract All the",
            "debug_pack.bin",
            "Open",
            true,
            [&]() { gui.CurrentPage = std::make_unique<ExtractPageDebug>(); }
        },
        {
            "BundleManagerButton",
            ImVec4(0.6f, 1.0f, 0.6f, 1.0f),
            ImVec4(0.0f, 1.0f, 0.0f, 1.0f),
            ImVec4(0.1f, 0.5f, 0.1f, 1.0f),
            "Used to browse and modify the",
            "Localization Bundles",
            "Coming soon",
            true,
            []() { /* gui.CurrentPage = std::make_unique<BundleManager>(); */ }
        }
    };

    if (!IsTIP()) {
        entries.push_back({
            "ShaderManagerButton",
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
            ImVec4(0.25f, 0.25f, 0.25f, 1.0f),
            "Used to browse and modify the",
            "Shader .wad",
            "Coming Soon",
            false,
            []() {}
        });
        entries.push_back({
            "SaveManagerButton",
            ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
            ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
            ImVec4(0.25f, 0.25f, 0.25f, 1.0f),
            "Used to browse and modify the",
            "Save Files",
            "Coming Soon",
            false,
            []() {}
        });
    }

    entries.push_back({
        "ExportManagerButton",
        ImVec4(0.4f, 0.4f, 0.4f, 1.0f),
        ImVec4(0.0f, 0.0f, 0.0f, 1.0f),
        ImVec4(0.25f, 0.25f, 0.25f, 1.0f),
        "Used to mass export the",
        ".pkg Files",
        "Extract",
        true,
        [&]() { gui.CurrentPage = std::make_unique<ExtractPage>(); }
    });

    // Render each entry as a vertical list item
    for (const auto& entry : entries) {
        ImGui::PushStyleColor(ImGuiCol_ChildBg, entry.bg_color);
        ImGui::PushStyleColor(ImGuiCol_Border, entry.border_color);

        // Remove scroll box by setting NoScrollbar and NoScrollWithMouse flags
        ImGuiWindowFlags child_flags = ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::BeginChild(entry.child_id, ImVec2(ImGui::GetContentRegionAvail().x, 120), true, child_flags);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));
        ImGui::TextWrapped("%s", entry.desc1);
        ImGui::Text("%s", entry.desc2);
        ImGui::PopStyleColor();

        ImGui::Dummy(ImVec2(0, 10)); // Add some vertical space

        ImGui::PushStyleColor(ImGuiCol_Button, entry.button_color);
        ImGui::BeginDisabled(!entry.enabled);
        if (ImGui::Button(entry.button_text, ImVec2(200, 50)) && entry.enabled) {
            entry.on_click();
        }
        ImGui::EndDisabled();
        ImGui::PopStyleColor();

        ImGui::EndChild();
        ImGui::PopStyleColor(2);

        ImGui::Spacing(); // Space between list items
    }

    ImGui::End();
}
