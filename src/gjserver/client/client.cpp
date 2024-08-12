#include "client.h"
#include "gjserver/logging/log.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TClient::TClient(
        std::shared_ptr<TLog> log,
        std::function<HandlerResolver> handler_resolver,
        std::function<DisconnectionProcessor> disconnection_processor,
        std::string addr,
        std::string port)
      : TService(log, 1)
      , m_addr(std::move(addr))
      , m_port(std::move(port))
      , m_resolver(m_context)
      , m_socket(m_context)
      , m_handlerResolver(handler_resolver)
      , m_disconnectionProcessor(disconnection_processor)
    {
    }

    //--------------------------------------------------------------//
    bool TClient::ResolveHandler(std::shared_ptr<TClientSession> session, const THeader* request) {
        return m_handlerResolver(session, request);
    }

    //--------------------------------------------------------------//
    bool TClient::ProcessDisconnect(std::shared_ptr<TClientSession> session) {
        return m_disconnectionProcessor(session);
    }

    //--------------------------------------------------------------//
    std::shared_ptr<TClientSession> TClient::Connect() {
        std::shared_ptr<TClientSession> session;

        asio::error_code error;
        asio::connect(m_socket, m_resolver.resolve(m_addr, m_port), error);
        if (error) {
            GJS_ERROR(std::move(std::string("Warning: could not connect : ").append(error.message())));
            return {};
        }
        session = std::shared_ptr<TClientSession>(new TClientSession(m_log, this, std::move(m_socket)));
        session->Read();

        return session;
    }

    //--------------------------------------------------------------//
    bool TClient::Reconnect(std::shared_ptr<TClientSession> session) {
        if (!session->Reconnect(m_resolver.resolve(m_addr, m_port))) {
            GJS_WARN("Warning: could not reconnect");
            return false;
        }

        session->Read();
        return true;
    }

    //////////////////////////////////////////////////////////////////

}  // namespace NGJServer