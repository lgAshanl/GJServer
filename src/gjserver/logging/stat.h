#pragma once

#include <queue>
#include <shared_mutex>
#include <string>
#include <string_view>
#include <unordered_map>

#include "gjserver/concurrency/service.h"
#include "gjserver/utils/time.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TStat : public TService {
        struct TKeyStat {
            uint32_t m_nread;
            uint32_t m_nwrite;
        };

    public:
        TStat(std::shared_ptr<TLog> log);

        ~TStat();

        void Start();

        void Stop();

        void LogAccess(const std::string& key, bool is_read);

        void ReportGlobalStat();

    private:
        void ClearStatFrame(TTimestamp now);

    private:
        mutable std::shared_mutex m_mutex;

        std::unordered_map<std::string, TKeyStat> m_globalStatByKey;

        TKeyStat m_globalStat;

        TKeyStat m_framedStat;

        // <time, is_read>
        std::queue<std::pair<TTimestamp, bool>> m_frame;

        TDuration m_frameDuration;

    public:
        static constexpr char DEFAULT_KV_DELIMITER = ' ';
        static constexpr TDuration DEFAULT_FRAME_DURATION = TDuration::Seconds(5);
    };

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
