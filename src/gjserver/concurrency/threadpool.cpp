#include "threadpool.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TThreadPool::TThreadPool(asio::io_context& io_context, size_t nthreads)
      : m_nthreads(nthreads)
      , m_context(io_context)
      , m_work(make_work_guard(io_context))
      , m_threads()
    { }

    //--------------------------------------------------------------//
    TThreadPool::~TThreadPool() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TThreadPool::Start() {
        for (size_t i = 0; i < m_nthreads; ++i) {
            m_threads.emplace_back(std::thread(
                [this]() {
                    m_context.run();
                }));
        }
    }

    //--------------------------------------------------------------//
    void TThreadPool::Stop() {
        assert(m_nthreads == m_threads.size() || 0 == m_threads.size());
        m_work.reset();
        while (m_threads.size()) {
            m_threads.back().join();
            m_threads.pop_back();
        }
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
