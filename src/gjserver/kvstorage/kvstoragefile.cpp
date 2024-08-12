#include "kvstoragefile.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TKVStorageFile::TKVStorageFile(std::shared_ptr<TLog> log, std::string&& filepath)
      : TLoggable(log)
      , m_filepath(std::move(filepath))
    { }

    //--------------------------------------------------------------//
    void TKVStorageFile::UpdateFile(std::unordered_map<std::string, std::string>&& map) {
        std::string new_version_path = m_filepath + ".new";
        if (!StoreFile(m_log, new_version_path, map) || !ChangeStorageFile(new_version_path)) {
            GJS_ERROR("KV file sync failed");
            return;
        }
    }

    //--------------------------------------------------------------//
    bool TKVStorageFile::LoadFile(
        std::shared_ptr<TLog> log,
        std::string_view path,
        std::unordered_map<std::string, std::string>& map) {
        auto m_log = log;
        try {
            std::ifstream input(path.data());
            if (!input.is_open()) {
                GJS_FATAL(std::move(std::string("failed open kv file: ").append(path)));
                throw std::exception();
            }
            for(std::string line; getline(input, line);) {
                map.emplace(SplitKeyValue(std::move(line)));
            }
        }
        catch (std::exception& exception) {
            std::string report;
            report.append("KV Storage file load failed. Filepath: ").append(path);
            report.append(" Error: ").append(exception.what());
            GJS_FATAL(std::move(report));
            throw;
        }

        return true;
    }

    //--------------------------------------------------------------//
    bool TKVStorageFile::StoreFile(
        std::shared_ptr<TLog> log,
        std::string_view path,
        const std::unordered_map<std::string, std::string>& map) {
        auto m_log = log;

        std::ofstream file;
        try {
            file.open(path.data(), std::ios_base::out & std::ios_base::binary);
            if (!file.is_open()) {
                GJS_ERROR(std::move(std::string("KV file create failed. Path: ").append(path)));
                return false;
            }
        
            for (const auto& item: map) {
                file << item.first << " " << item.second << std::endl;
            }
        }
        catch (const std::exception& except) {
            GJS_ERROR(std::move(std::string("KV file write failed.").append(except.what())));
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------//
    std::pair<std::string, std::string> TKVStorageFile::SplitKeyValue(std::string&& line) {
        const auto pos = line.find(KV_DELIMITER);
        if (std::string::npos == pos)
            return { line, std::string() };

        std::string value = line.substr(pos + 1, line.size());

        line.resize((std::min)(pos, line.size()));
        line.shrink_to_fit();
        value.shrink_to_fit();
        return { std::move(line), std::move(value) };
    }

    //--------------------------------------------------------------//
    bool TKVStorageFile::ChangeStorageFile(const std::string& new_storage_file) {
        std::string tmp_filepath = m_filepath + ".old";
        std::error_code error;
        std::filesystem::rename(m_filepath, tmp_filepath, error);
        if (error) {
            GJS_ERROR(std::move(std::string("KV old file rename failed. Errno: ").append(error.message())));
            return false;
        }

        std::filesystem::rename(new_storage_file, m_filepath, error);
        if (error) {
            GJS_ERROR(std::move(std::string("KV file rename failed. Errno: ").append(error.message())));
            return false;
        }

        if (!std::filesystem::remove(tmp_filepath)) {
            GJS_ERROR("KV old file rm failed.");
            return false;
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
