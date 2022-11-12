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

#ifndef PACKAGE_V666_HPP
#define PACKAGE_V666_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "../defines.hpp"

namespace dravex::v666
{
    struct diskpkgfileinfo_t
    {
        uint32_t string_offset;
        uint32_t checksum;
        uint32_t size_compressed;
        uint32_t data_offset;
        uint32_t size_decompressed;
        uint8_t is_compressed;
        uint8_t padding00[3];
        uint32_t unknown00;
        uint32_t unknown01;
        uint32_t unknown02;
        uint32_t unknown03;
        uint32_t unknown04;
    };

} // namespace dravex::v666

#endif // PACKAGE_V666_HPP
