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

#ifndef UTILS_HPP
#define UTILS_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "defines.hpp"
#include "zlib.h"

namespace dravex::utils
{
    /**
     * Inflates the given compressed input data using zlib.
     *
     * @param {uint8_t*} input - The input data to inflate.
     * @param {std::size_t} input_size - The input data length.
     * @param {std::size_t} offset - The input data offset to start the inflating at.
     * @param {std::vector&} output - The output vector to hold the inflated data.
     * @return {bool} True on success, false otherwise.
     */
    static bool inflate(const uint8_t* input, const std::size_t input_size, const std::size_t offset, std::vector<uint8_t>& output)
    {
        // Initialize the zlib stream..
        z_stream zstream{};
        if (inflateInit(&zstream) != Z_OK)
            return false;

        // Prepare the stream input..
        zstream.avail_in = input_size - offset;
        zstream.next_in  = const_cast<uint8_t*>(input + offset);

        std::vector<uint8_t> chunk(1024, '\0');

        do
        {
            // Prepare the output chunk..
            zstream.avail_out = 1024;
            zstream.next_out  = chunk.data();

            // Inflate the input data..
            switch (inflate(&zstream, Z_NO_FLUSH))
            {
                case Z_NEED_DICT:
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    inflateEnd(&zstream);
                    return false;
            }

            // Copy the inflated data to the output vector..
            const auto size = 1024 - zstream.avail_out;
            output.insert(output.end(), chunk.begin(), chunk.begin() + size);

        } while (zstream.avail_out == 0);

        // Cleanup the zlib stream..
        inflateEnd(&zstream);
        return true;
    }

    /**
     * Parses a block of null terminated strings into a map.
     *
     * @param {std::vector&} input - The input data to parse.
     * @param {std::map&} output - The output map to store the parsed strings.
     */
    static void parse_strings(const std::vector<char>& input, std::map<uint32_t, std::string>& output)
    {
        output.clear();

        std::size_t offset = 0;
        while (offset < input.size())
        {
            const char* str = static_cast<const char*>(input.data() + offset);
            if (str == nullptr || *str == '\0')
                break;

            output[offset] = str;
            offset += ::strlen(str) + 1;
        }
    }

} // namespace dravex::utils

#endif // UTILS_HPP
