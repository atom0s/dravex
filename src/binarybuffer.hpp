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

#ifndef BINARYBUFFER_HPP
#define BINARYBUFFER_HPP

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "defines.hpp"

namespace dravex
{
    namespace detail
    {
        template<typename>
        struct is_list_type : std::false_type
        {};

        template<typename T>
        struct is_list_type<std::list<T>> : std::true_type
        {};

        template<typename>
        struct is_map_type : std::false_type
        {};

        template<typename K, typename V>
        struct is_map_type<std::map<K, V>> : std::true_type
        {};

        template<typename>
        struct is_string_type : std::false_type
        {};

        template<>
        struct is_string_type<std::string> : std::true_type
        {};

        template<typename>
        struct is_vector_type : std::false_type
        {};

        template<typename T>
        struct is_vector_type<std::vector<T>> : std::true_type
        {};

    } // namespace detail

    class binarybuffer final
    {
    public:
        enum endianess
        {
            little,
            big
        };

    private:
        std::vector<uint8_t> data_;
        std::size_t index_;
        endianess endian_ = endianess::little;

        template<typename T>
        void swap(T* val)
        {
            union
            {
                T input_;
                uint8_t buffer_[sizeof(T)];
            } s{}, d{};

            s.input_ = *val;

            for (std::size_t x = 0; x < sizeof(T); x++)
                d.buffer_[x] = s.buffer_[sizeof(T) - x - 1];

            *val = d.input_;
        }

        void check_size(const std::size_t needed_size) noexcept
        {
            const auto size = this->data_.size();

            // Check if the buffer is empty or initially too small..
            if (size == 0 || size < needed_size)
            {
                this->data_.resize(size + needed_size);
                return;
            }

            // Check if the buffer position can fit the data without overflowing..
            if (this->index_ + needed_size > size)
            {
                this->data_.resize(size + needed_size);
                return;
            }
        }

        template<typename T>
        T read_list(void)
        {
            const auto size = this->read_trivial<std::size_t>();

            T value{};
            for (std::size_t x = 0; x < size; x++)
                value.push_back(this->read_trivial<typename T::value_type>());

            return value;
        }

        template<typename T>
        T read_list(const std::size_t count)
        {
            T value{};
            for (std::size_t x = 0; x < count; x++)
                value.push_back(this->read_trivial<typename T::value_type>());

            return value;
        }

        template<typename T>
        T read_map(void)
        {
            const auto size = this->read_trivial<std::size_t>();

            T value{};
            for (std::size_t x = 0; x < size; x++)
            {
                auto& val = value[this->read<typename T::key_type>()];
                val       = this->read<typename T::mapped_type>();
            }

            return value;
        }

        template<typename T>
        T read_map(const std::size_t count)
        {
            T value{};
            for (std::size_t x = 0; x < count; x++)
            {
                auto& val = value[this->read<typename T::key_type>()];
                val       = this->read<typename T::mapped_type>();
            }

            return value;
        }

        template<typename T>
        T read_string(void)
        {
            std::string value;

            char v{};
            while ((v = this->data_.at(this->index_++)) != '\0')
                value.push_back(v);

            return value;
        }

        template<typename T>
        T read_string(const std::size_t size)
        {
            // Validate the read index..
            if (this->index_ > this->data_.size())
                throw std::exception("invalid read string attempt");
            if (this->index_ + size > this->data_.size())
                throw std::exception("invalid read string attempt");

            // Read the data into a temporary buffer first to avoid invalid string size from 00 padding..
            std::vector<char> buffer((const char*)this->data_.data() + this->index_, (const char*)this->data_.data() + this->index_ + size);
            buffer.push_back('\0');

            // Get the actual string length..
            const auto ssize = strlen(buffer.data());
            if (ssize > size)
                throw std::exception("invalid read string attempt");

            // Create the new string from the buffer data..
            std::string value = buffer.data();

            this->index_ += size;

            return value;
        }

        template<typename T>
        T read_trivial(void)
        {
            const auto size = sizeof(T);

            // Validate the read index..
            if (this->index_ > this->data_.size())
                throw std::exception("invalid read attempt");
            if (this->index_ + size > this->data_.size())
                throw std::exception("invalid read attempt");

            T value{};
            std::memcpy(&value, this->data_.data() + this->index_, size);
            this->index_ += size;

            if (this->endian_ == endianess::big)
                this->swap<T>(&value);

            return value;
        }

        template<typename T>
        T read_vector(void)
        {
            const auto size = this->read_trivial<std::size_t>();

            T value{};
            for (std::size_t x = 0; x < size; x++)
                value.push_back(this->read_trivial<typename T::value_type>());

            return value;
        }

        template<typename T>
        T read_vector(const std::size_t count)
        {
            T value{};
            for (std::size_t x = 0; x < count; x++)
                value.push_back(this->read_trivial<typename T::value_type>());

            return value;
        }

        template<typename T>
        void write_list(T value)
        {
            // Write the element count..
            this->write<std::size_t>(value.size());

            // Write the list values..
            for (const auto& v : value)
                this->write<typename T::value_type>(v);
        }

        template<typename T>
        void write_map(T value)
        {
            // Write the element count..
            this->write<std::size_t>(value.size());

            // Write the map key-value pairs..
            for (const auto& [k, v] : value)
            {
                this->write<typename T::key_type>(k);
                this->write<typename T::mapped_type>(v);
            }
        }

        template<typename T>
        void write_string(T value)
        {
            const auto size = value.size() + 1;
            this->check_size(size);

            std::memcpy(this->data_.data() + this->index_, value.data(), size);

            this->index_ += size;
        }

        template<typename T>
        void write_trivial(T value)
        {
            const auto size = sizeof(value);
            this->check_size(size);

            if (this->endian_ == endianess::big)
                this->swap<T>(&value);

            std::memcpy(this->data_.data() + this->index_, (uint8_t*)&value, size);

            this->index_ += size;
        }

        template<typename T>
        void write_vector(T value)
        {
            // Write the element count..
            this->write<std::size_t>(value.size());

            // Write the values..
            for (const auto& v : value)
                this->write<typename T::value_type>(v);
        }

    public:
        binarybuffer(void)
            : index_(0)
        {}
        binarybuffer(const uint8_t* data, const std::size_t size, const std::size_t starting_index = 0)
            : data_(data, data + size)
            , index_(starting_index)
        {}
        ~binarybuffer(void)
        {}

        template<typename T, std::enable_if_t<detail::is_list_type<T>::value>* = nullptr>
        T read(void)
        {
            return this->read_list<T>();
        }

        template<typename T, std::enable_if_t<detail::is_list_type<T>::value>* = nullptr>
        T read(const std::size_t count)
        {
            return this->read_list<T>(count);
        }

        template<typename T, std::enable_if_t<detail::is_map_type<T>::value>* = nullptr>
        T read(void)
        {
            return this->read_map<T>();
        }

        template<typename T, std::enable_if_t<detail::is_map_type<T>::value>* = nullptr>
        T read(const std::size_t count)
        {
            return this->read_map<T>(count);
        }

        template<typename T, std::enable_if_t<detail::is_string_type<T>::value>* = nullptr>
        T read(void)
        {
            return this->read_string<T>();
        }

        template<typename T, std::enable_if_t<detail::is_string_type<T>::value>* = nullptr>
        T read(const std::size_t size)
        {
            return this->read_string<T>(size);
        }

        template<typename T, std::enable_if_t<std::conjunction_v<std::is_trivial<T>, std::is_standard_layout<T>>>* = nullptr>
        T read(void)
        {
            return this->read_trivial<T>();
        }

        template<typename T, std::enable_if_t<detail::is_vector_type<T>::value>* = nullptr>
        T read(void)
        {
            return this->read_vector<T>();
        }

        template<typename T, std::enable_if_t<detail::is_vector_type<T>::value>* = nullptr>
        T read(const std::size_t count)
        {
            return this->read_vector<T>(count);
        }

        template<typename T, std::enable_if_t<detail::is_list_type<T>::value>* = nullptr>
        void write(T value)
        {
            this->write_list<T>(value);
        }

        template<typename T, std::enable_if_t<detail::is_map_type<T>::value>* = nullptr>
        void write(T value)
        {
            this->write_map<T>(value);
        }

        template<typename T, std::enable_if_t<detail::is_string_type<T>::value>* = nullptr>
        void write(T value)
        {
            this->write_string<T>(value);
        }

        template<typename T, std::enable_if_t<std::conjunction_v<std::is_trivial<T>, std::is_standard_layout<T>>>* = nullptr>
        void write(T value)
        {
            this->write_trivial<T>(value);
        }

        template<typename T, std::enable_if_t<detail::is_vector_type<T>::value>* = nullptr>
        void write(T value)
        {
            this->write_vector<T>(value);
        }

        void write_raw(const uint8_t* data, const std::size_t size)
        {
            this->check_size(size);

            std::memcpy(this->data_.data() + this->index_, data, size);

            this->index_ += size;
        }

    public:
        uint8_t* data(void) noexcept
        {
            return this->data_.data();
        }

        const std::size_t size(void) noexcept
        {
            return this->data_.size();
        }

        const std::size_t index(void) noexcept
        {
            return this->index_;
        }

        void clear(void) noexcept
        {
            this->reset();

            this->data_.clear();
        }

        void reset(void) noexcept
        {
            this->index_ = 0;
        }

        const std::size_t set_index(const std::size_t index) noexcept
        {
            const auto prev = this->index_;
            this->index_    = index;

            return prev;
        }

        endianess get_endianess(void) const noexcept
        {
            return this->endian_;
        }

        void set_endianess(const endianess endian) noexcept
        {
            this->endian_ = endian;
        }
    };

} // namespace dravex

#endif // BINARYBUFFER_HPP
