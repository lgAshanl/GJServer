#pragma once

#include <memory>
#include <future>

#include "gjserver/client/client.h"
#include "gjserver/client/clientconfig.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    enum class TClientError : int {
        EOK             = 0,
        EUNEXP          = 1,
        EDISCON_SUCCESS = 2,
        EDISCON_FAILED  = 3,
    };

    //////////////////////////////////////////////////////////////////
    class TGJClient : public TLoggable {
    public:
        TGJClient(std::string addr, std::string port);

        ~TGJClient();

        void Start();

        void Stop();

        bool TestConnect();

        void ProcessFile(std::string path);

        void ProcessFileSlow(std::string path);

    private:

        bool ResolveHandler(std::shared_ptr<TClientSession> session, const THeader* message);

        bool ProcessDisconnect(std::shared_ptr<TClientSession> session);

        std::shared_ptr<TClientSession> Connect();

        void RequestRead(std::shared_ptr<TClientSession> session, std::string&& key);

        void RequestWrite(std::shared_ptr<TClientSession> session, std::string&& key, std::string&& value);

        bool ProcessCommand(std::shared_ptr<TClientSession> session, const TClientConfig::TCommandPair& command);

    private:
        TClient m_client;

        std::promise<std::pair<TClientError, std::string>> m_promise;

    public:

        static constexpr uint32_t DEFAULT_RECONNECT_ATTEMPTS_COUNT = 5;
        static constexpr uint32_t DEFAULT_RECONNECT_TIMEOUT_SECONDS = 1;
    };

    //////////////////////////////////////////////////////////////////
} // namespace NGJServer
