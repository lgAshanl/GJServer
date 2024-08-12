#include "time.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TDuration& TDuration::operator+=(const TDuration& other) {
        m_microseconds += other.m_microseconds;
        return *this;
    }

    //--------------------------------------------------------------//
    TDuration& TDuration::operator-=(const TDuration& other) {
        m_microseconds -= other.m_microseconds;
        return *this;
    }

    //--------------------------------------------------------------//
    bool TDuration::operator<(const TDuration& duration) {
        return m_microseconds < duration.m_microseconds;
    }

    //--------------------------------------------------------------//
    TDuration::operator std::chrono::microseconds() const {
        return std::chrono::microseconds(m_microseconds);
    }

    //--------------------------------------------------------------//
    uint64_t TDuration::Milliseconds() const {
        return m_microseconds / 1000;
    }

    //--------------------------------------------------------------//
    uint64_t TDuration::Microseconds() const {
        return m_microseconds;
    }

    //////////////////////////////////////////////////////////////////
    TTimestamp::TTimestamp(std::chrono::time_point<std::chrono::steady_clock> time)
        : m_value(time) { }

    //--------------------------------------------------------------//
    TTimestamp::TTimestamp(uint64_t microseconds)
        : m_value(std::chrono::steady_clock::time_point(std::chrono::microseconds(microseconds))) { }

    //--------------------------------------------------------------//
    TDuration TTimestamp::operator-(const TTimestamp& previous) const {
        return TDuration(
            std::chrono::duration_cast<std::chrono::microseconds>(m_value - previous.m_value).count());
    }

    //--------------------------------------------------------------//
    uint64_t TTimestamp::Microseconds() {
        return std::chrono::duration_cast<std::chrono::microseconds>(m_value.time_since_epoch()).count();
    }

    //--------------------------------------------------------------//
    TTimestamp TTimestamp::Now() {
        return TTimestamp(std::chrono::steady_clock::now());
    }

    //--------------------------------------------------------------//
    TTimestamp TTimestamp::FromMicroseconds(uint64_t microseconds) {
        return TTimestamp(microseconds);
    }

}  // namespace NGJServer