#include "gjclient.h"

#include "gjserver/kvstorage/kvstoragefile.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TGJClient::TGJClient(std::string addr, std::string port)
      : TLoggable(std::shared_ptr<TLog>(new TLog()))
      , m_client(
            m_log,
            [this] (std::shared_ptr<TClientSession> session, const THeader* response) {
                return ResolveHandler(session, response);
            },
            std::bind(&TGJClient::ProcessDisconnect, this, std::placeholders::_1),
            addr,
            port)
    { }

    //--------------------------------------------------------------//
    TGJClient::~TGJClient() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TGJClient::Start() {
        m_log->Start();
        m_client.Start();
    }

    //--------------------------------------------------------------//
    void TGJClient::Stop() {
        m_client.Stop();
        m_log->Stop();
    }

    //--------------------------------------------------------------//
    bool TGJClient::TestConnect() {
        std::shared_ptr<TClientSession> session = Connect();
        if (!session) {
            return false;
        }

        return true;
    }

    //--------------------------------------------------------------//
    void TGJClient::ProcessFile(std::string path) {
        std::shared_ptr<TClientSession> session = Connect();
        if (!session) {
            return;
        }

        std::vector<TClientConfig::TCommandPair> commands;
        TClientConfig::LoadFile(m_log, path, commands);

        for (const auto& item : commands) {
            while (!ProcessCommand(session, item));
        }
    }

    //--------------------------------------------------------------//
    void TGJClient::ProcessFileSlow(std::string path) {
        std::shared_ptr<TClientSession> session = Connect();
        if (!session) {
            return;
        }

        std::vector<TClientConfig::TCommandPair> commands;
        TClientConfig::LoadFile(m_log, path, commands);

        for (const auto& item : commands) {
            while (!ProcessCommand(session, item));
            std::this_thread::sleep_for(std::chrono::microseconds(TDuration::MilliSeconds(100)));
        }
    }

    //--------------------------------------------------------------//
    bool TGJClient::ResolveHandler(std::shared_ptr<TClientSession> session, const THeader* message) {
        (void)session;

        if (!message->VerifyResponse()) {
            GJS_ERROR(std::move(
                std::string("Recieved invalid cmd code: ").append(std::to_string(message->m_cmd))));
            m_promise.set_value({TClientError::EUNEXP, {}});
            return false;
        }

        std::string response_val;
        if (ECMD::READ_RESPONSE == message->m_cmd) {
            response_val = message->ContentView();
        }

        m_promise.set_value({TClientError::EOK, std::move(response_val)});
        return true;
    }

    //--------------------------------------------------------------//
    bool TGJClient::ProcessDisconnect(std::shared_ptr<TClientSession> session) {
        uint32_t attempt_count = DEFAULT_RECONNECT_ATTEMPTS_COUNT;
        for (; attempt_count && !m_client.Reconnect(session); --attempt_count) {
            std::this_thread::sleep_for(std::chrono::seconds(DEFAULT_RECONNECT_TIMEOUT_SECONDS));
        }

        if (!attempt_count) {
            GJS_ERROR("Failed all reconnections to server");
            m_promise.set_value({ TClientError::EDISCON_FAILED, {} });
        }

        GJS_LOG("CLIENT RECONNECTED");
        m_promise.set_value({ TClientError::EDISCON_SUCCESS, {} });

        return true;
    }

    //--------------------------------------------------------------//
    std::shared_ptr<TClientSession> TGJClient::Connect() {
        std::shared_ptr<TClientSession> session;
        for (uint32_t i = 0; i < DEFAULT_RECONNECT_ATTEMPTS_COUNT; ++i) {
            session = m_client.Connect();
            if (session) {
                return session;
            }
        }

        GJS_ERROR("Failed all connections to server");

        return nullptr;
    }

    //--------------------------------------------------------------//
    void TGJClient::RequestRead(std::shared_ptr<TClientSession> session, std::string&& key) {
        auto request(THeader::MakeCommand(ECMD::READ_REQUEST, key.data(), key.size()));

        session->Write(std::move(request));
    }

    //--------------------------------------------------------------//
    void TGJClient::RequestWrite(std::shared_ptr<TClientSession> session, std::string&& key, std::string&& value) {
        const uint32_t total_size = key.size() + 1 + value.size();
        key.reserve(total_size);
        key.append(" ");
        key.append(std::move(value));

        auto request(THeader::MakeCommand(ECMD::WRITE_REQUEST, key.data(), key.size()));

        session->Write(std::move(request));
    }

    //--------------------------------------------------------------//
    bool TGJClient::ProcessCommand(std::shared_ptr<TClientSession> session, const TClientConfig::TCommandPair& command) {
        m_promise = decltype(m_promise)();
        switch (command.m_cmd) {
        case ECMD::READ_REQUEST:
            RequestRead(session, std::string(command.m_args));
            break;
        case ECMD::WRITE_REQUEST:
            {
                auto pair = TKVStorageFile::SplitKeyValue(std::string(command.m_args));
                RequestWrite(session, std::move(pair.first), std::move(pair.second));
            }
            break;
        default:
            GJS_ERROR(std::move(std::string("Invalid command: ").append(command.ToString())));
            m_promise.set_value({ TClientError::EUNEXP, {}});
            throw std::runtime_error("Invalid command");
        }

        auto ret = m_promise.get_future().get();
        switch (ret.first) {
        case TClientError::EOK:
            if (ECMD::READ_REQUEST == command.m_cmd) {
                std::string report;
                report.reserve(command.m_args.size() + ret.second.size() + 32);
                report.append("<<< Client: Key: ");
                report.append(command.m_args);
                report.append(" Value: ");
                report.append(ret.second);
                report.append(">>>\n");

                GJS_STDOUT(std::move(report));
            }
            return true;
        case TClientError::EDISCON_SUCCESS:
            // retry command
            return false;
        case TClientError::EDISCON_FAILED:
            GJS_FATAL("Connection totally lost");
            throw std::runtime_error("connection lost");
        case TClientError::EUNEXP:
        default:
            GJS_FATAL("Unexpected fatal");
            throw std::runtime_error("unexpected");
        }
    }

    //////////////////////////////////////////////////////////////////
} // namespace NGJServer
