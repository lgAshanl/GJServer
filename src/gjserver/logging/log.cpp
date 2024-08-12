#include "log.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TLog::TLog()
      : m_context()
      , m_threadpool(m_context, 1)
      , m_streamLog(std::cout)
      , m_streamError(std::cerr)
      , m_streamStdOut(std::cout)
    { }

    //--------------------------------------------------------------//
    TLog::~TLog() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TLog::Start() {
        m_threadpool.Start();
    }

    //--------------------------------------------------------------//
    void TLog::Stop() {
        m_context.stop();
        m_threadpool.Stop();
    }

    //--------------------------------------------------------------//
    void TLog::Log(EStream stream, std::string&& report) {
        MakeEndl(report);
        std::basic_ostream<char>& outstream = SwitchStream(stream);
        m_context.post(
            [&outstream, report = std::move(report)]() mutable {
                outstream << report;
            });
    }

    //--------------------------------------------------------------//
    void TLog::LogSync(EStream stream, std::string&& report) {
        MakeEndl(report);
        std::basic_ostream<char>& outstream = SwitchStream(stream);

        auto future = asio::post(m_context,
            std::packaged_task<void()>(
                [&outstream, report = std::move(report)]() mutable {
                    outstream << report;
                }));
        future.wait();
    }

    //--------------------------------------------------------------//
    void TLog::MakeEndl(std::string& str) {
        if (str.size() && '\n' != str.back())
            str.append("\n");
    }

    //////////////////////////////////////////////////////////////////
    TLoggable::TLoggable(std::shared_ptr<TLog> log)
      : m_log(log)
    { }

    //--------------------------------------------------------------//
    void TLoggable::Log(TLog::EStream stream, std::string&& report) {
        m_log->Log(stream, std::move(report));
    }

    //--------------------------------------------------------------//
    void TLoggable::LogSync(TLog::EStream stream, std::string&& report) {
        m_log->LogSync(stream, std::move(report));
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
