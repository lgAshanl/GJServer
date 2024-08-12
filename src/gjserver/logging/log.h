#pragma once

#include <asio.hpp>

#include <string>
#include <iostream>
#include <future>
#include <ostream>

#include "gjserver/concurrency/threadpool.h"

#include <asio/thread_pool.hpp>

namespace NGJServer {

    //////////////////////////////////////////////////////////////////

#define GJS_LOG(message) (m_log->Log(TLog::EStream::LOGSTREAM, message));
#define GJS_WARN(message) (m_log->Log(TLog::EStream::LOGSTREAM, message));
#define GJS_ERROR(message) (m_log->LogSync(TLog::EStream::ERRORSTREAM, message));
#define GJS_FATAL(message) (m_log->LogSync(TLog::EStream::ERRORSTREAM, message));
#define GJS_STDOUT(message) (m_log->LogSync(TLog::EStream::STDOUTSTREAM, message));

    //////////////////////////////////////////////////////////////////

    class TLog : public std::enable_shared_from_this<TLog> {
    public:
        enum class EStream : short {
            LOGSTREAM    = 0,
            ERRORSTREAM  = 1,
            STDOUTSTREAM = 2
        };

    public:
        TLog();

        ~TLog();

        void Start();

        void Stop();

        void Log(EStream stream, std::string&& report);

        void LogSync(EStream stream, std::string&& report);

        static void MakeEndl(std::string& str);

    private:

        inline std::basic_ostream<char>& SwitchStream(EStream stream) const;

    private:
        asio::io_context m_context;

        TThreadPool m_threadpool;

        std::basic_ostream<char>& m_streamLog;

        std::basic_ostream<char>& m_streamError;

        std::basic_ostream<char>& m_streamStdOut;
    };

    //--------------------------------------------------------------//
    std::basic_ostream<char>& TLog::SwitchStream(EStream stream) const {
        switch (stream)
        {
        case EStream::LOGSTREAM:
            return m_streamLog;
        case EStream::STDOUTSTREAM:
            return m_streamStdOut;
        default:
            return m_streamError;
        }
    }

    //////////////////////////////////////////////////////////////////
    class TLoggable {
    public:
        TLoggable(std::shared_ptr<TLog> log);

        void Log(TLog::EStream stream, std::string&& report);

        void LogSync(TLog::EStream stream, std::string&& report);

    public:
        const std::shared_ptr<TLog> m_log;
    };

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
