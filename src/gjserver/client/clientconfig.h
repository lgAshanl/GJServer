#pragma once

#include <memory>
#include <future>

#include "gjserver/net/proto.h"
#include "gjserver/logging/log.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    class TClientConfig {
    public:
        struct TCommandPair {
            ECMD m_cmd;
            std::string m_args;

            bool operator==(const TCommandPair& other) const;

            std::string ToString() const;

            static TCommandPair Parse(std::string_view line);
        };

    public:

        static bool LoadFile(
            std::shared_ptr<TLog> log,
            std::string_view path,
            std::vector<TCommandPair>& comands);

        static bool StoreFile(
            std::shared_ptr<TLog> log,
            std::string_view path,
            const std::vector<TCommandPair>& comands);
    };

    //////////////////////////////////////////////////////////////////
} // namespace NGJServer
