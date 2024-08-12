#pragma once

#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "gjserver/concurrency/service.h"
#include "gjserver/kvstorage/kvstoragefile.h"
#include "gjserver/logging/stat.h"
#include "gjserver/utils/time.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TKVStorage : public TService {
    public:
        TKVStorage(std::shared_ptr<TLog> log, std::string&& filepath);

        ~TKVStorage();

        void Start();

        void Stop();

        bool Load(std::string_view path);

        bool Get(const std::string& key, std::string& value);

        void Set(std::string&& key, std::string&& value);

    private:

        void UpdateFile();

    private:
        TStat m_stat;

        mutable std::shared_mutex m_mutex;

        std::unordered_map<std::string, std::string> m_map;

        std::atomic<int> m_isMapDirty;

        TKVStorageFile m_file;

    public:

        static constexpr TDuration DEFAULT_SYNC_PERIOD = TDuration::MilliSeconds(100);
    };

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
