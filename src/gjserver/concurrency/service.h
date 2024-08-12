#pragma once

#include <atomic>
#include <thread>

#include "threadpool.h"
#include "gjserver/logging/log.h"
#include "gjserver/utils/time.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TService : public TLoggable {
    public:
        template<class F>
        struct TServiceTimedTask : public std::enable_shared_from_this<TServiceTimedTask<F>> {
            TServiceTimedTask(TService* service, TDuration period, F&& task);

            void Exec() const;

            void ExecOptional() const;
        private:
            TService* const m_service;
            const TDuration m_period;
            mutable std::atomic<uint64_t> m_execTimeMicroSec = 0;
            const F m_task;
        };

    public:
        TService(std::shared_ptr<TLog> log, uint32_t nthreads);

        ~TService();

        void Start();

        void Stop();

        template<class... F>
        void EnqueueTask(F&&... f);

        template<class F>
        void EnqueueTimedTaskTask(TDuration period, F&& f);

        template<class F>
        void EnqueueOptionalTimedTaskTask(TDuration period, F&& f);

    protected:

        asio::io_context m_context;

        TThreadPool m_threadpool;
    };

    //--------------------------------------------------------------//
    template<class... F>
    void TService::EnqueueTask(F&&... f){
        m_context.post(std::forward<F>(f)...);
    }

    //--------------------------------------------------------------//
    template<class F>
    void TService::EnqueueTimedTaskTask(TDuration period, F&& f){
        std::shared_ptr<TServiceTimedTask<F>> task{new TServiceTimedTask<F>(this, period, std::move(f))};
        m_context.post([task] () { task->Exec(); });
    }

    //--------------------------------------------------------------//
    template<class F>
    void TService::EnqueueOptionalTimedTaskTask(TDuration period, F&& f){
        std::shared_ptr<TServiceTimedTask<F>> task{new TServiceTimedTask<F>(this, period, std::move(f))};
        m_context.post([task] () { task->ExecOptional(); });
    }

    //////////////////////////////////////////////////////////////////
    template<class F>
    TService::TServiceTimedTask<F>::TServiceTimedTask(TService* service, TDuration period, F&& task)
      : m_service(service)
      , m_period(period)
      , m_execTimeMicroSec(0)
      , m_task(std::move(task))
    { }

    //--------------------------------------------------------------//
    template<class F>
    void TService::TServiceTimedTask<F>::Exec() const {
        auto self(TServiceTimedTask<F>::shared_from_this());

        TTimestamp now = TTimestamp::Now();
        TTimestamp exec_time = TTimestamp::FromMicroseconds(m_execTimeMicroSec.load(std::memory_order_relaxed));
        TDuration diff = now - exec_time;
        while (diff < m_period) {
            auto sleep_time = m_period - diff;
            std::this_thread::sleep_for(std::chrono::microseconds(sleep_time));
            now = TTimestamp::Now();
            diff = now - exec_time;
        }
        m_execTimeMicroSec.store(now.Microseconds(), std::memory_order_relaxed);
        m_task();
        m_service->m_context.post([self]() { self->Exec(); } );
    }

    //--------------------------------------------------------------//
    template<class F>
    void TService::TServiceTimedTask<F>::ExecOptional() const {
        auto self(TServiceTimedTask<F>::shared_from_this());

        TTimestamp now = TTimestamp::Now();
        TTimestamp exec_time = TTimestamp::FromMicroseconds(m_execTimeMicroSec.load(std::memory_order_relaxed));
        TDuration diff = now - exec_time;
        if (diff < m_period) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        else {
            m_execTimeMicroSec.store(now.Microseconds(), std::memory_order_relaxed);
            m_task();
        }
        m_service->m_context.post([self]() { self->ExecOptional(); } );
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer
