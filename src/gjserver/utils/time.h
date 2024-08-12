#pragma once

#include <chrono>

namespace NGJServer {

/*
           ／＞    フ
           |  _  _|
          ／`ミ＿xノ
        /       |
       /   ヽ   ﾉ
      │    | | |
  ／￣|     | | |
  | (￣ヽ＿_ヽ_)__)
  ＼二つ
*/

    //////////////////////////////////////////////////////////////////
    class TDuration {
    public:
        constexpr inline TDuration(uint64_t microseconds = 0);

        TDuration& operator+=(const TDuration& other);

        TDuration& operator-=(const TDuration& other);

        bool operator<(const TDuration& duration);

        operator std::chrono::microseconds() const;

        uint64_t Milliseconds() const;

        uint64_t Microseconds() const;

        static constexpr inline TDuration Seconds(uint64_t seconds);

        static constexpr inline TDuration MilliSeconds(uint64_t milliseconds);

    private:
        uint64_t m_microseconds;
    };

    //--------------------------------------------------------------//
    constexpr TDuration::TDuration(uint64_t microseconds)
        : m_microseconds(microseconds) { }

    //--------------------------------------------------------------//
    constexpr TDuration TDuration::Seconds(uint64_t seconds) {
        return TDuration(seconds * 1000000);
    }

    //--------------------------------------------------------------//
    constexpr TDuration TDuration::MilliSeconds(uint64_t milliseconds) {
        return TDuration(milliseconds * 1000);
    }

    //////////////////////////////////////////////////////////////////
    inline TDuration operator-(const TDuration& l, const TDuration& r) {
        return TDuration(l) -= r;
    }

    //////////////////////////////////////////////////////////////////
    class TTimestamp {
    public:
        TTimestamp(std::chrono::time_point<std::chrono::steady_clock> time);

        TTimestamp(uint64_t microseconds);

        TDuration operator-(const TTimestamp& previous) const;

        uint64_t Microseconds();

        static TTimestamp Now();

        static TTimestamp FromMicroseconds(uint64_t microseconds);

    private:
        std::chrono::time_point<std::chrono::steady_clock> m_value;
    };

}  // namespace NGJServer