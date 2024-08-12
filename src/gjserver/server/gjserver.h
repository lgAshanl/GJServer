#pragma once

#include <memory>

#include "gjserver/server/server.h"
#include "gjserver/kvstorage/kvstorage.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TGJServer : public TLoggable {
    public:
        TGJServer(short port, std::string&& dict_path);

        ~TGJServer();

        void Start();

        void Stop();

        bool ResolveHandler(std::shared_ptr<TServerSession> session, const THeader* response);

    private:
        TServer m_server;

        TKVStorage m_storage;

    public:

        static constexpr uint32_t DEFAULT_NTHREADS = 4;
    };

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer
