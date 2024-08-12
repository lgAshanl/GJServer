#pragma once

#include <list>
#include <thread>

#include <asio/thread_pool.hpp>
#include <asio/post.hpp>
#include <asio/io_context.hpp>

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TThreadPool {
    public:
        TThreadPool(asio::io_context& io_context, size_t threads);

        ~TThreadPool();

        void Start();

        void Stop();

        template<class... F>
        void EnqueueTask(F&&... f);

    private:
        const uint32_t m_nthreads;
        asio::io_context& m_context;

        asio::executor_work_guard<asio::io_context::executor_type> m_work;
        std::list<std::thread> m_threads;
    };

    //--------------------------------------------------------------//
    template<class... F>
    void TThreadPool::EnqueueTask(F&&... f){
        m_context.post(std::forward<F>(f)...);
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
