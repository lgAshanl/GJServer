#include "server.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TServer::TServer(
        std::shared_ptr<TLog> log,
        std::function<HandlerResolver> handler,
        short port,
        uint32_t nthreads)
      : TService(log, nthreads)
      , m_acceptor(m_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), port))
      , m_socket(m_context)
      , m_handlerResolver(handler)
    {
        m_context.post(std::bind(&TServer::Accept, this));
    }

    //--------------------------------------------------------------//
    bool TServer::ResolveHandler(std::shared_ptr<TServerSession> session, const THeader* request) {
        return m_handlerResolver(session, request);
    }

    //--------------------------------------------------------------//
    void TServer::ProcessDisconnect(std::shared_ptr<TServerSession> session) {
        (void)session;
    };

    //--------------------------------------------------------------//
    void TServer::Accept() {
        m_acceptor.async_accept(
            m_socket,
            [this](asio::error_code ec) {
                if (!ec) {
                    std::shared_ptr<TServerSession> session(
                        new TServerSession(m_log, this, std::move(m_socket)));
                    session->Read();
                }
                else {
                    GJS_ERROR(std::move(std::string("Failed accept error: ").append(ec.message())));
                }

                Accept();
            });
    }

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer
