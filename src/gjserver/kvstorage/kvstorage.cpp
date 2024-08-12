#include "kvstorage.h"

#include "kvstoragefile.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TKVStorage::TKVStorage(std::shared_ptr<TLog> log, std::string&& filepath)
      : TService(log, 1)
      , m_stat(log)
      , m_mutex()
      , m_map()
      , m_isMapDirty()
      , m_file(log, std::move(filepath))
    {
        EnqueueTimedTaskTask(DEFAULT_SYNC_PERIOD, [this]() { UpdateFile(); });
    }

    //--------------------------------------------------------------//
    TKVStorage::~TKVStorage() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TKVStorage::Start() {
        TService::Start();
        m_stat.Start();
        Load(m_file.m_filepath);
    }

    //--------------------------------------------------------------//
    void TKVStorage::Stop() {
        TService::Stop();
    }

    //--------------------------------------------------------------//
    bool TKVStorage::Load(std::string_view path) {
        std::lock_guard<decltype(m_mutex)> lock(m_mutex);
        return TKVStorageFile::LoadFile(m_log, path, m_map);
    }

    //--------------------------------------------------------------//
    bool TKVStorage::Get(const std::string& key, std::string& value) {
        m_stat.LogAccess(key, true);

        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        const auto iter = m_map.find(key);
        if (m_map.end() == iter) {
            return false;
        }

        value = iter->second;
        lock.unlock();

        return true;
    }

    //--------------------------------------------------------------//
    void TKVStorage::Set(std::string&& key, std::string&& value) {
        m_stat.LogAccess(key, false);

        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        m_map.insert_or_assign(std::move(key), std::move(value));
        lock.unlock();

        m_isMapDirty.store(1, std::memory_order_relaxed);
    }

    //--------------------------------------------------------------//
    void TKVStorage::UpdateFile() {
        if (!m_isMapDirty.load(std::memory_order_relaxed)) {
            return;
        }

        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        auto map = m_map;
        lock.unlock();

        m_file.UpdateFile(std::move(map));
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
