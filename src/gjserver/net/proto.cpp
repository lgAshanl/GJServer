#include <cstring>
#include <string_view>
#include <memory>

#include "proto.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    bool THeader::VerifyVersion() const {
        return 0 == m_version;
    }

    //--------------------------------------------------------------//
    bool THeader::VerifyRequest() const {
        return VerifyVersion() && (0x10 && m_cmd);
    }

    //--------------------------------------------------------------//
    bool THeader::VerifyResponse() const {
        return VerifyVersion() && (0x20 && m_cmd);
    }

    //--------------------------------------------------------------//
    std::string_view THeader::ContentView() const {
        return std::string_view(reinterpret_cast<const char*>(this) + sizeof(THeader), m_size - sizeof(THeader));
    }

    //--------------------------------------------------------------//
    void THeader::Prepare(ECMD cmd, const char* content, uint32_t content_size) {
        m_version = 0;
        m_cmd = cmd;
        m_size = content_size + sizeof(THeader);

        if (content_size) {
            char* data = reinterpret_cast<char*>(this) + sizeof(THeader);
            std::memcpy(data, content, content_size);
        }
    }

    //--------------------------------------------------------------//
    std::unique_ptr<char[]> THeader::MakeCommand(ECMD cmd, const char* content, uint32_t content_size) {
        const uint32_t size = content_size + sizeof(THeader);
        char* buf = new char[size];
        THeader* command = reinterpret_cast<THeader*>(buf);

        if (!command)
            return nullptr;
        command->Prepare(cmd, content, content_size);
        return std::unique_ptr<char[]>(reinterpret_cast<char*>(command));
    }
} // namespace NGJServer
