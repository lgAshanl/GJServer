#pragma once

#include <memory>
#include <functional>

#include <asio/ts/internet.hpp>

#include "gjserver/concurrency/service.h"
#include "gjserver/net/proto.h"
#include "gjserver/net/session.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TServer;

    //////////////////////////////////////////////////////////////////
    using TServerSession = TSession<TServer>;

    //////////////////////////////////////////////////////////////////
    class TServer : public TService {
    public:
        using HandlerResolver = TServerSession::HandlerResolver;
        using DisconnectionProcessor = TServerSession::DisconnectionProcessor;

    public:
        TServer(
            std::shared_ptr<TLog> log,
            std::function<HandlerResolver> handler,
            short port,
            uint32_t nthreads = DEFAULT_NTHREADS);

        bool ResolveHandler(std::shared_ptr<TServerSession> session, const THeader* request);

        void ProcessDisconnect(std::shared_ptr<TServerSession> session);

    private:

        void Accept();

    private:
        asio::ip::tcp::acceptor m_acceptor;
        asio::ip::tcp::socket m_socket;

        std::function<HandlerResolver> m_handlerResolver;
    
    public:
        static constexpr uint32_t DEFAULT_NTHREADS = 4;
    };

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer
