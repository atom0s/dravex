/**
 * dravex - Copyright (c) 2022 atom0s [atom0s@live.com]
 *
 * Contact: https://www.atom0s.com/
 * Contact: https://discord.gg/UmXNvjq
 * Contact: https://github.com/atom0s
 * Support: https://paypal.me/atom0s
 * Support: https://patreon.com/atom0s
 * Support: https://github.com/sponsors/atom0s
 *
 * This file is part of dravex.
 *
 * dravex is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * dravex is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Affero General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with dravex.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "logging.hpp"
#include "imgui.h"
#include "imgui_fontawesome.hpp"

/**
 * Constructor and Destructor
 */
dravex::logging::logging(void)
    : autoscroll_{true}
    , autoscroll_new_entries_{true}
    , scroll_{false}
    , clear_{false}
{}
dravex::logging::~logging(void)
{}

/**
 * Returns the singleton instance of this class.
 *
 * @return {logging&} The singleton instance of this class.
 */
dravex::logging& dravex::logging::instance(void)
{
    static dravex::logging log;
    return log;
}

/**
 * Clears the current log.
 */
void dravex::logging::clear(void)
{
    this->log_.clear();
}

/**
 * Logs the given message.
 *
 * @param {dravex::loglevel&} level - The level of the message being logged.
 * @param {std::string&} message - The message to log.
 */
void dravex::logging::log(const dravex::loglevel level, const std::string& message)
{
    // Add the message to the log..
    this->log_.push_back(std::make_tuple(level, message));

    // Mark the log to scroll due to a new entry..
    if (this->autoscroll_new_entries_)
        this->scroll_ = true;
}

/**
 * Renders the log via ImGui.
 */
void dravex::logging::render(void)
{
    if (ImGui::BeginTable("##dravex_log_table", 2, ImGuiTableFlags_BordersH | ImGuiTableFlags_BordersV | ImGuiTableFlags_BordersInnerH | ImGuiTableFlags_BordersInnerV | ImGuiTableFlags_BordersOuterH | ImGuiTableFlags_BordersOuterV | ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY | ImGuiTableFlags_RowBg | ImGuiTableFlags_ContextMenuInBody | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_NoSavedSettings, ImVec2(0.0f, 0.0f), 0.0f))
    {
        ImGui::TableSetupColumn("##icon", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Message", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupScrollFreeze(0, 1);
        ImGui::TableHeadersRow();

        ImGuiListClipper clipper;
        clipper.Begin(this->log_.size());

        while (clipper.Step())
        {
            for (auto x = clipper.DisplayStart; x < clipper.DisplayEnd; x++)
            {
                const auto& l = this->log_[x];

                // Prepare the column sizes..
                ImGui::TableNextRow();
                if (x == clipper.DisplayStart)
                {
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(16);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::PushItemWidth(FLT_MAX);
                }

                // Prepare the log entry icon..
                std::string icon_str;
                switch (std::get<0>(l))
                {
                    case dravex::loglevel::none:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        icon_str += " ";
                        break;
                    case dravex::loglevel::critical:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.3f, 0.3f, 1.0f));
                        icon_str += ICON_FA_SKULL_CROSSBONES;
                        break;
                    case dravex::loglevel::error:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.5f, 0.5f, 1.0f));
                        icon_str += ICON_FA_SKULL_CROSSBONES;
                        break;
                    case dravex::loglevel::warn:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.9f, 0.4f, 1.0f));
                        icon_str += ICON_FA_TRIANGLE_EXCLAMATION;
                        break;
                    case dravex::loglevel::info:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.4f, 1.0f, 1.0f, 1.0f));
                        icon_str += ICON_FA_CIRCLE_INFO;
                        break;
                    case dravex::loglevel::debug:
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.75f, 0.6f, 1.0f, 1.0f));
                        icon_str += ICON_FA_BUG;
                        break;
                }

                ImGui::PushID(x);
                ImGui::TableSetColumnIndex(0);
                ImGui::Text(icon_str.c_str());
                ImGui::TableSetColumnIndex(1);
                ImGui::Text(std::format("{}", std::get<1>(l)).c_str());
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.0f, 8.0f));
                if (ImGui::BeginPopupContextItem("##dravex_log_popup"))
                {
                    ImGui::Checkbox("Auto-scroll?", &this->autoscroll_);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Scrolls the log automatically as new entries are added if\nthe log is already scrolled to the bottom.");
                    ImGui::Checkbox("Auto-scroll on new entries?", &this->autoscroll_new_entries_);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Scrolls the log to the newest entry each time one is added.");
                    ImGui::Separator();
                    if (ImGui::MenuItem("Copy Current"))
                        ImGui::SetClipboardText(std::get<1>(l).c_str());
                    if (ImGui::MenuItem("Copy Table"))
                    {
                        std::string full_log;
                        for (const auto& line : this->log_)
                            full_log += std::format("{}\n", std::get<1>(line));
                        ImGui::SetClipboardText(full_log.c_str());
                    }
                    ImGui::Separator();
                    if (ImGui::MenuItem("Clear Log"))
                        this->clear_ = true;
                    ImGui::EndPopup();
                }
                ImGui::PopStyleVar();
                ImGui::PopStyleColor();
                ImGui::PopID();
                ImGui::PopStyleColor();
            }
        }

        clipper.End();

        // Handle auto-scrolling..
        if (this->scroll_ || (this->autoscroll_ && (ImGui::GetScrollY() >= ImGui::GetScrollMaxY())))
        {
            ImGui::SetScrollHereY(0.0f);
            this->scroll_ = false;
        }

        ImGui::EndTable();
    }

    // Clear the log if flagged..
    if (this->clear_)
    {
        this->clear_ = false;
        this->log_.clear();
    }
}
