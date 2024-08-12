#pragma once

#include <fstream>
#include <filesystem>
#include <string>
#include <string_view>
#include <unordered_map>

#include "gjserver/logging/log.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TKVStorageFile : public TLoggable {
    public:
        TKVStorageFile(std::shared_ptr<TLog> log, std::string&& filepath);

        void UpdateFile(std::unordered_map<std::string, std::string>&& map);

        static bool LoadFile(
            std::shared_ptr<TLog> log,
            std::string_view path,
            std::unordered_map<std::string, std::string>& map);

        static bool StoreFile(
            std::shared_ptr<TLog> log,
            std::string_view path,
            const std::unordered_map<std::string, std::string>& map);

        static std::pair<std::string, std::string> SplitKeyValue(std::string&& line);

    private:

        bool ChangeStorageFile(const std::string& new_storage_file);

    public:

        const std::string m_filepath;

    public:
        static constexpr char KV_DELIMITER = ' ';
    };

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
