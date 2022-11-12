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

#include "package.hpp"
#include "v118.hpp"
#include "v666.hpp"
#include "../utils.hpp"

/**
 * Constructor and Destructor
 */
dravex::package::package(void)
    : pkg_file_{nullptr}
{}
dravex::package::~package(void)
{}

/**
 * Parses client version v118 package files.
 * 
 * @param {std::shared_ptr} buffer - The current binary buffer object holding the index file data.
 * @return {bool} True on success, false otherwise.
 */
bool dravex::package::parse_v118(std::shared_ptr<dravex::binarybuffer> buffer)
{
    // Read the header information..
    const auto version             = buffer->read<uint32_t>();
    this->guid_                    = buffer->read<std::vector<uint8_t>>(16);
    const auto entry_count         = buffer->read<uint32_t>();
    const auto entry_offset        = buffer->read<uint32_t>();
    const auto string_table_size   = buffer->read<uint32_t>();
    const auto string_table_offset = buffer->read<uint32_t>();

    // Open the data file for reading..
    FILE* f = nullptr;
    if (::fopen_s(&this->pkg_file_, this->pkg_path_.string().c_str(), "rb") != ERROR_SUCCESS)
        return false;

    // Read and validate the game.pkg file GUID..
    uint8_t data_guid[16]{};
    if (::fread(data_guid, 1, 16, this->pkg_file_) != 16 || std::memcmp(this->guid_.data(), data_guid, 16) != 0)
    {
        ::fclose(this->pkg_file_);
        return false;
    }

    // Read the file entries..
    buffer->set_index(entry_offset);
    const auto entries = buffer->read<std::vector<v118::diskpkgfileinfo_t>>(entry_count);

    // Read the string table information..
    buffer->set_index(string_table_offset);
    const auto string_table_data = buffer->read<std::vector<char>>(string_table_size);

    // Parse the string table..
    dravex::utils::parse_strings(string_table_data, this->strings_);
    if (this->strings_.size() != entry_count)
        return false;

    // Convert the entries into generalized type..
    for (auto x = 0; x < entry_count; x++)
    {
        const auto& e = entries[x];

        this->entries_.push_back({
            static_cast<uint32_t>(e.flags) & 0x3F,
            e.string_offset,
            e.data_offset,
            e.size_compressed,
            e.size_decompressed,
            e.checksum_compressed,
            (e.flags & 0x40) == 0x40,
        });
    }

    return true;
}

/**
 * Parses client version v666 package files.
 * 
 * @param {std::shared_ptr} buffer - The current binary buffer object holding the index file data.
 * @return {bool} True on success, false otherwise.
 */
bool dravex::package::parse_v666(std::shared_ptr<dravex::binarybuffer> buffer)
{
    // Read the uncompressed header information..
    const auto version1 = buffer->read<uint32_t>();
    const auto version2 = buffer->read<uint32_t>();
    this->guid_         = buffer->read<std::vector<uint8_t>>(16);

    // Open the data file for reading..
    FILE* f = nullptr;
    if (::fopen_s(&this->pkg_file_, this->pkg_path_.string().c_str(), "rb") != ERROR_SUCCESS)
        return false;

    // Read and validate the game.pkg file GUID..
    uint8_t data_guid[16]{};
    if (::fread(data_guid, 1, 16, this->pkg_file_) != 16 || std::memcmp(this->guid_.data(), data_guid, 16) != 0)
    {
        ::fclose(this->pkg_file_);
        return false;
    }

    // Decompress the remaining index data..
    std::vector<uint8_t> data_decompressed;
    if (!dravex::utils::inflate(buffer->data(), buffer->size(), buffer->index(), data_decompressed))
        return false;

    // Set the binary buffer to the decompressed data..
    buffer = std::make_shared<dravex::binarybuffer>(data_decompressed.data(), data_decompressed.size());

    // Read the file type sizes table..
    std::vector<uint32_t> blocks(21, '\0');
    for (auto x = 0; x < blocks.size(); x++)
        blocks[x] = buffer->read<uint32_t>();

    // Calculate the total entry count..
    const auto entry_count = std::reduce(blocks.begin(), blocks.end());

    // Read the file entries..
    const auto entries = buffer->read<std::vector<v666::diskpkgfileinfo_t>>(entry_count);

    // Read the string table information..
    const auto string_table_size = buffer->read<uint32_t>();
    const auto string_table_data = buffer->read<std::vector<char>>(string_table_size);

    // Parse the string table..
    dravex::utils::parse_strings(string_table_data, this->strings_);
    if (this->strings_.size() != entry_count)
        return false;

    /**
     * Returns the file type for the given file index.
     *
     * @param {int32_t} index - The index of the file.
     * @return {uint32_t} The file type.
     */
    const auto get_file_type = [&blocks](const int32_t index) -> uint32_t {
        for (auto x = 0, value = 0; x < blocks.size(); x++, value += blocks[x])
        {
            if (index < value)
                return x;
        }
        return 22;
    };

    // Convert the entries into generalized type..
    for (auto x = 0; x < entry_count; x++)
    {
        const auto& e = entries[x];

        this->entries_.push_back({
            get_file_type(x),
            e.string_offset,
            e.data_offset,
            e.size_compressed,
            e.size_decompressed,
            e.checksum,
            e.is_compressed > 0,
        });
    }

    return true;
}

/**
 * Returns the singleton instance of this class.
 *
 * @return {package&} The singleton instance of this class.
 */
dravex::package& dravex::package::instance(void)
{
    static dravex::package pkg;
    return pkg;
}

/**
 * Returns the file extension for the given file type.
 *
 * @param {uint32_t} file_type - The file type.
 * @return {const char*} The file extension.
 */
const char* dravex::package::get_extension(const uint32_t file_type)
{
    return file_type >= _countof(dravex::extensions)
               ? ""
               : dravex::extensions[file_type];
}

/**
 * Opens and parses the given game assets archive.
 *
 * @param {std::string&} path - The path to the game.pki index file to open for parsing.
 * @return {bool} True on success, false otherwise.
 */
bool dravex::package::open(const std::string& path)
{
    this->close();

    std::error_code ec;
    if (!std::filesystem::exists(path, ec) || ec)
        return false;

    std::filesystem::path pki_path = path;
    if (!std::filesystem::exists(pki_path, ec) || ec)
        return false;

    std::filesystem::path pkg_path = pki_path.parent_path();
    pkg_path /= "game.pkg";

    if (!std::filesystem::exists(pkg_path, ec) || ec)
        return false;

    this->pki_path_ = pki_path;
    this->pkg_path_ = pkg_path;

    // Open the index file for reading..
    FILE* f = nullptr;
    if (::fopen_s(&f, pki_path.string().c_str(), "rb") != ERROR_SUCCESS)
        return false;

    // Obtain the index file size..
    ::fseek(f, 0, SEEK_END);
    const auto size = ::_ftelli64(f);
    ::fseek(f, 0, SEEK_SET);

    // Read the index file data..
    std::vector<uint8_t> data(size, '\0');
    ::fread(data.data(), 1, size, f);
    ::fclose(f);

    // Create a binary buffer to parse the file data..
    auto buffer = std::make_shared<dravex::binarybuffer>(data.data(), data.size());

    // Handle the file based on the version..
    switch (buffer->read<uint32_t>())
    {
        case 2: // Client Version: v118
            buffer->reset();
            return this->parse_v118(buffer);

        case 3: // Client Version: v666
            buffer->reset();
            return this->parse_v666(buffer);

        default:
            break;
    }

    return false;
}

/**
 * Closes the current archive, cleaning up its resources.
 */
void dravex::package::close(void)
{
    if (this->pkg_file_ != nullptr)
    {
        ::fclose(this->pkg_file_);
        this->pkg_file_ = nullptr;
    }

    this->pki_path_.clear();
    this->pkg_path_.clear();
    this->guid_.clear();
    this->entries_.clear();
    this->strings_.clear();
}

/**
 * Returns the count of parsed file entries.
 *
 * @return {std::size_t} The count of file entries.
 */
std::size_t dravex::package::get_entry_count(void)
{
    return this->entries_.size();
}

/**
 * Returns the file entry at the given index.
 *
 * @param {int32_t} index - The index of the file entry to return.
 * @return {fileentry_t&} The file entry.
 */
dravex::fileentry_t& dravex::package::get_entry(const int32_t index)
{
    dravex::fileentry_t default_entry{};
    return (this->entries_.size() == 0 || index > this->entries_.size() || index == -1)
               ? default_entry
               : this->entries_[index];
}

/**
 * Returns the data for the given file entry.
 *
 * @param {int32_t} index - The file index to obtain the data of.
 * @return {std::vector} The file data.
 */
std::vector<uint8_t> dravex::package::get_entry_data(const int32_t index)
{
    if (this->entries_.size() == 0 || index > this->entries_.size() || index == -1)
        return {};

    const auto& e = this->entries_[index];
    auto size     = e.is_compressed_ ? e.size_compressed_ : e.size_uncompressed_;

    // Read the file data from the game.pkg file..
    std::vector<uint8_t> data(size, '\0');
    ::fseek(this->pkg_file_, e.data_offset_, SEEK_SET);
    ::fread(data.data(), size, 1, this->pkg_file_);

    // Return the raw data if it is not compressed..
    if (!e.is_compressed_)
        return data;

    // Inflate the data via zlib..
    std::vector<uint8_t> data_decompressed;
    if (!dravex::utils::inflate(data.data(), data.size(), 0, data_decompressed))
        return {};

    return data_decompressed;
}

/**
 * Returns the string that starts at the given table offset.
 * 
 * @param {uint32_t} offset - The string table offset of the string to return.
 * @return {const char*} The string on success, nullptr otherwise.
 */
const char* dravex::package::get_string(const uint32_t offset)
{
    const auto iter = this->strings_.find(offset);
    return iter == this->strings_.end()
               ? nullptr
               : iter->second.c_str();
}
