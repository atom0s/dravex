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

#ifndef ARCHIVE_V118_HPP
#define ARCHIVE_V118_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"

namespace dravex::v118
{
    struct diskpkgfileinfo_t
    {
        uint32_t string_offset_;
        uint8_t flags_;
        uint8_t padding00_[3];
        uint32_t unknown00_;
        uint32_t unknown01_;
        uint32_t unknown02_;
        uint32_t data_offset_;
        uint32_t checksum_decompressed_;
        uint32_t size_decompressed_;
        uint32_t checksum_compressed_;
        uint32_t size_compressed_;
    };

} // namespace dravex::v118

#endif // ARCHIVE_V118_HPP
