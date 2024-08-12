#pragma once

#include <cstdint>

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    enum ECMD: uint16_t {
        None = 0,
        READ_REQUEST = 0x11,
        WRITE_REQUEST = 0x12,
        READ_RESPONSE = 0x21,
        WRITE_RESPONSE = 0x22,
    };

    //////////////////////////////////////////////////////////////////
#pragma pack(push, 1)
    struct THeader {
        uint16_t m_version;

        ECMD m_cmd;

        uint32_t m_size;

        bool VerifyVersion() const;

        bool VerifyRequest() const;

        bool VerifyResponse() const;

        std::string_view ContentView() const;

        void Prepare(ECMD cmd, const char* content, uint32_t content_size);

        static std::unique_ptr<char[]> MakeCommand(ECMD cmd, const char* content, uint32_t content_size);

        static void operator delete (void *p) {
            return delete[] static_cast<char*>(p);
        }
    };
#pragma pack(pop)

} // namespace NGJServer
