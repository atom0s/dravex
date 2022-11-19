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

#ifndef LOGGING_HPP
#define LOGGING_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "defines.hpp"

namespace dravex
{
    enum class loglevel : int32_t
    {
        none     = 0,
        critical = 1,
        error    = 2,
        warn     = 3,
        info     = 4,
        debug    = 5,
    };

    class logging final
    {
        logging(logging const&)            = delete;
        logging(logging&&)                 = delete;
        logging& operator=(logging const&) = delete;
        logging& operator=(logging&&)      = delete;

        logging(void);
        ~logging(void);

        mutable std::mutex mutex_;
        std::vector<std::tuple<dravex::loglevel, std::string>> log_;
        bool autoscroll_;
        bool autoscroll_new_entries_;
        bool scroll_;
        bool clear_;

    public:
        static logging& instance(void);

        void clear(void);
        void log(const dravex::loglevel level, const std::string& message);

        void render(void);
    };

} // namespace dravex

#endif // LOGGING_HPP
