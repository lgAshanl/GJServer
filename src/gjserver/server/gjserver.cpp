#include "gjserver.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    TGJServer::TGJServer(short port, std::string&& dict_path)
      : TLoggable(std::shared_ptr<TLog>(new TLog()))
      ,  m_server(
            m_log,
            [this] (std::shared_ptr<TServerSession> session, const THeader* response) {
                return ResolveHandler(session, response);
            },
            port,
            DEFAULT_NTHREADS)
      , m_storage(m_log, std::move(dict_path))
    { }

    //--------------------------------------------------------------//
    TGJServer::~TGJServer() {
        Stop();
    }

    //--------------------------------------------------------------//
    void TGJServer::Start() {
        m_log->Start();
        m_storage.Start();
        m_server.Start();
    }

    //--------------------------------------------------------------//
    void TGJServer::Stop() {
        m_server.Stop();
        m_storage.Stop();
        m_log->Stop();
    }

    //--------------------------------------------------------------//
    bool TGJServer::ResolveHandler(std::shared_ptr<TServerSession> session, const THeader* request) {
        if (!request->VerifyRequest()) {
            GJS_ERROR(std::move(
                std::string("Recieved invalid cmd code: ").append(std::to_string(request->m_cmd))));
            return false;
        }

        // TODO: in real world here should be async future wait for kvstorage response
        if (ECMD::READ_REQUEST == request->m_cmd) {
            std::string key(request->ContentView());
            std::string value;
            m_storage.Get(key, value);

            auto response(THeader::MakeCommand(ECMD::READ_RESPONSE, value.data(), value.size()));
            session->Write(std::move(response));
        }
        else if (ECMD::WRITE_REQUEST == request->m_cmd) {
            auto pair = TKVStorageFile::SplitKeyValue(std::string(request->ContentView()));
            m_storage.Set(std::move(pair.first), std::move(pair.second));

            auto response(THeader::MakeCommand(ECMD::WRITE_RESPONSE, nullptr, 0));
            session->Write(std::move(response));
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////

} // namespace NGJServer
