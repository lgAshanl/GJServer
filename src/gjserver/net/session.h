#pragma once

#include <asio.hpp>

#include "gjserver/net/proto.h"
#include "gjserver/logging/log.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    template<class HandlerProcessor>
    class TSession : public std::enable_shared_from_this<TSession<HandlerProcessor>>, public TLoggable {
    public:
        // ret ~ continue process
        typedef bool (HandlerResolver) (std::shared_ptr<TSession> session, const THeader* request);
        typedef bool (DisconnectionProcessor)(std::shared_ptr<TSession>);
        
    public:
        TSession(
            std::shared_ptr<TLog> log,
            HandlerProcessor* handler_processor,
            asio::ip::tcp::socket socket,
            uint32_t max_message_size = DEFAULT_MAX_MESSAGE_SIZE);

        ~TSession();

        void Write(std::unique_ptr<char[]> message);

        void Read();

        bool Reconnect(asio::ip::basic_resolver_results<asio::ip::tcp> resolve);

    private:
        HandlerProcessor* m_handlerProcessor;

        const uint32_t m_maxMessageSize;
        asio::ip::tcp::socket m_socket;
        std::vector<char> m_buffer;
        THeader* m_header;
        uint32_t m_bufferSize;

    public:
        static constexpr uint32_t DEFAULT_MAX_MESSAGE_SIZE = 4096;
    };

    //--------------------------------------------------------------//
    template<class HandlerProcessor>
    TSession<HandlerProcessor>::TSession(
        std::shared_ptr<TLog> log,
        HandlerProcessor* handler_processor,
        asio::ip::tcp::socket socket,
        uint32_t max_message_size)
      : TLoggable(log)
      , m_handlerProcessor(handler_processor)
      , m_maxMessageSize(max_message_size)
      , m_socket(std::move(socket))
      , m_buffer(max_message_size)
      , m_header(reinterpret_cast<THeader*>(m_buffer.data()))
      , m_bufferSize(0)
    { }

    //--------------------------------------------------------------//
    template<class HandlerProcessor>
    TSession<HandlerProcessor>::~TSession() {
        m_socket.close();
    }

    //--------------------------------------------------------------//
    template<class HandlerProcessor>
    void TSession<HandlerProcessor>::Read() {
        uint32_t readsize = 0;
        if (m_bufferSize < sizeof(THeader)) {
            readsize = sizeof(THeader) - m_bufferSize;
        }
        else {
            assert(m_bufferSize < m_header->m_size);
            readsize = m_header->m_size - m_bufferSize;
        }
        char* buf = m_buffer.data() + m_bufferSize;

        auto self(TSession::shared_from_this());
        m_socket.async_read_some(
            asio::buffer(buf, readsize),
            [this, self](asio::error_code ec, std::size_t size) {
                if (ec) {
                    if (asio::error::eof != ec) {
                        GJS_ERROR(std::move(std::string("Receive error: ").append(ec.message())));
                    }
                    GJS_LOG("disconnect");
                    m_handlerProcessor->ProcessDisconnect(self);
                    return;
                }

                m_bufferSize += size;
                if (m_bufferSize >= sizeof(THeader) && m_bufferSize == m_header->m_size) {
                    if (!m_handlerProcessor->ResolveHandler(self, m_header)) {
                        // disconnect on error
                        GJS_WARN("Failed handler processing");
                        GJS_LOG("disconnect client");
                        m_handlerProcessor->ProcessDisconnect(self);
                        return;
                    }
                    m_bufferSize = 0;
                }

                Read();
            });
    }

    //--------------------------------------------------------------//
    template<class HandlerProcessor>
    void TSession<HandlerProcessor>::Write(std::unique_ptr<char[]> message) {
        auto self(TSession::shared_from_this());
        THeader* header = reinterpret_cast<THeader*>(message.get());
        asio::async_write(
            m_socket,
            asio::buffer(header, header->m_size),
            [this, self, message = std::move(message)](asio::error_code ec, std::size_t) {
                THeader* header = reinterpret_cast<THeader*>(message.get());
                (void)header;
                if (ec) {
                    GJS_ERROR(std::move(std::string("Send error: ").append(ec.message())));
                    m_handlerProcessor->ProcessDisconnect(self);
                    return;
                }
            });
    }

    //--------------------------------------------------------------//
    template<class HandlerProcessor>
    bool TSession<HandlerProcessor>::Reconnect(asio::ip::basic_resolver_results<asio::ip::tcp> resolve) {
        m_socket.close();

        asio::error_code error;
        asio::connect(m_socket, resolve, error);
        if (error) {
            GJS_WARN(std::move(std::string("Failed reconnect: ").append(error.message())));
            return false;
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer
