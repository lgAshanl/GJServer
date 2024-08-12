#include "stat.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TStat::TStat(std::shared_ptr<TLog> log)
      : TService(log, 1)
      , m_globalStatByKey()
      , m_globalStat()
      , m_framedStat()
      , m_frame()
      , m_frameDuration(DEFAULT_FRAME_DURATION)
    {
        EnqueueOptionalTimedTaskTask(m_frameDuration, std::bind(&TStat::ReportGlobalStat, this));
    }

    //--------------------------------------------------------------//
    TStat::~TStat() {
    }

    //--------------------------------------------------------------//
    void TStat::Start() {
        TService::Start();
    }

    //--------------------------------------------------------------//
    void TStat::Stop() {
        TService::Stop();
    }

    //--------------------------------------------------------------//
    void TStat::LogAccess(const std::string& key, bool is_read) {
        std::unique_lock<decltype(m_mutex)> lock(m_mutex);
        TTimestamp now = TTimestamp::Now();
        ClearStatFrame(now);

        auto& global_stat_by_key = m_globalStatByKey[key];

        if (is_read) {
            ++global_stat_by_key.m_nread;
            ++m_globalStat.m_nread;
            ++m_framedStat.m_nread;
        }
        else {
            ++global_stat_by_key.m_nwrite;
            ++m_globalStat.m_nwrite;
            ++m_framedStat.m_nwrite;
        }

        TKeyStat global_stat_by_key_clone = global_stat_by_key;

        m_frame.emplace(now, is_read);

        lock.unlock();

        std::string report;
        report.reserve(key.size() + 32);
        report.append("$$$ SERVER Key: ").append(key).append(" ###\n");
        report.append("Reads: ").append(std::to_string(global_stat_by_key_clone.m_nread)).append("\n");
        report.append("Writes: ").append(std::to_string(global_stat_by_key_clone.m_nwrite)).append("\n");

        GJS_LOG(std::move(report));
    }

    //--------------------------------------------------------------//
    void TStat::ReportGlobalStat() {
        std::unique_lock<decltype(m_mutex)> unique_lock(m_mutex);
        TTimestamp now = TTimestamp::Now();
        ClearStatFrame(now);
        unique_lock.unlock();

        std::shared_lock<decltype(m_mutex)> lock(m_mutex);
        const auto global_stat = m_globalStat;
        const auto frame_stat  = m_framedStat;
        lock.unlock();

        std::string report;
        report.reserve(128);
        report.append("### SERVER: Frame stat ###\n");
        report.append("Reads: ").append(std::to_string(frame_stat.m_nread)).append("\n");
        report.append("Writes: ").append(std::to_string(frame_stat.m_nwrite)).append("\n");
        report.append("### Global stat ###\n");
        report.append("Reads: ").append(std::to_string(global_stat.m_nread)).append("\n");
        report.append("Writes: ").append(std::to_string(global_stat.m_nwrite)).append("\n");

        GJS_LOG(std::move(report));
    }

    //--------------------------------------------------------------//
    void TStat::ClearStatFrame(TTimestamp now) {
        while (m_frame.size()) {
            const auto& front = m_frame.front();
            auto diff = now - front.first;
            if (diff < m_frameDuration) {
                return;
            }

            const bool is_read = front.second;
            m_frame.pop();

            if (is_read)
                --m_framedStat.m_nread;
            else
                --m_framedStat.m_nwrite;
        }
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
