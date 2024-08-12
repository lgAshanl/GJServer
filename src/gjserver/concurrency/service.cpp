#include "service.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TService::TService(std::shared_ptr<TLog> log, uint32_t nthreads)
      : TLoggable(log)
      , m_context()
      , m_threadpool(m_context, nthreads)
    { }

    //--------------------------------------------------------------//
    TService::~TService() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TService::Start() {
        m_threadpool.Start();
    }

    //--------------------------------------------------------------//
    void TService::Stop() {
        m_context.stop();
        m_threadpool.Stop();
        m_log->Stop();
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
