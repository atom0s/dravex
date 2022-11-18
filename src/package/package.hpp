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

#ifndef PACKAGE_HPP
#define PACKAGE_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"
#include "../binarybuffer.hpp"

namespace dravex
{
    constexpr const char* extensions[] = {
        ".tga", ".bmp", ".dds", ".ttf", ".cobj",
        ".d3d", ".dre", ".wav", ".ogg", ".win",
        ".msc", ".mig", ".dict", ".gc", ".tile",
        ".world", ".zone", ".dat", ".fx", ".cfg",
        ".txt", ""};

    struct fileentry_t
    {
        uint32_t index_;
        uint32_t file_type_;
        uint32_t string_offset_;
        uint32_t data_offset_;
        uint32_t size_compressed_;
        uint32_t size_uncompressed_;
        uint32_t checksum_;
        bool is_compressed_;
    };

    class package final
    {
        package(package const&)            = delete;
        package(package&&)                 = delete;
        package& operator=(package const&) = delete;
        package& operator=(package&&)      = delete;

        package(void);
        ~package(void);

        std::filesystem::path pki_path_;
        std::filesystem::path pkg_path_;
        FILE* pkg_file_;

        std::vector<uint8_t> guid_;
        std::vector<std::shared_ptr<fileentry_t>> entries_;
        std::map<uint32_t, std::string> strings_;

        auto parse_v118(std::shared_ptr<dravex::binarybuffer> buffer) -> bool;
        auto parse_v666(std::shared_ptr<dravex::binarybuffer> buffer) -> bool;

    public:
        static package& instance(void);
        static const char* get_extension(const uint32_t file_type);

    public:
        auto open(const std::string& path) -> bool;
        auto close(void) -> void;

        auto get_entry_count(void) -> std::size_t;
        auto get_entry(const int32_t index) -> std::shared_ptr<dravex::fileentry_t>;
        auto get_entry_data(const int32_t index) -> std::vector<uint8_t>;
        auto get_string(const uint32_t offset) -> const char*;
    };

} // namespace dravex

#endif // PACKAGE_HPP
