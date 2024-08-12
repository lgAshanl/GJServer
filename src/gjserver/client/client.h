#pragma once

#include <memory>
#include <functional>

#include <asio.hpp>

#include "gjserver/concurrency/service.h"
#include "gjserver/net/proto.h"
#include "gjserver/net/session.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TClient;

    //////////////////////////////////////////////////////////////////
    using TClientSession = TSession<TClient>;

    //////////////////////////////////////////////////////////////////
    class TClient : public TService {
    public:
        // ret ~ continue process
        //typedef bool (Handler)(std::shared_ptr<TClientSession> session, const THeader* request, uint32_t size);
        using HandlerResolver = TClientSession::HandlerResolver;
        using DisconnectionProcessor = TClientSession::DisconnectionProcessor;

    public:
        TClient(
            std::shared_ptr<TLog> log,
            std::function<HandlerResolver> handler_resolver,
            std::function<DisconnectionProcessor> disconnection_processor,
            std::string addr,
            std::string port);

        std::shared_ptr<TClientSession> Connect();

        bool Reconnect(std::shared_ptr<TClientSession> session);

        bool ResolveHandler(std::shared_ptr<TClientSession> session, const THeader* request);

        bool ProcessDisconnect(std::shared_ptr<TClientSession> session);

    private:
        std::string m_addr;
        std::string m_port;

        asio::ip::tcp::resolver m_resolver;
        asio::ip::tcp::socket m_socket;

        std::function<HandlerResolver> m_handlerResolver;
        std::function<DisconnectionProcessor> m_disconnectionProcessor;
    
    public:

        static constexpr uint32_t DEFAULT_RECONNECT_ATTEMPTS_COUNT = 5;
    };

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer