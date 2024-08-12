#include "gjclient.h"

#include "gjserver/kvstorage/kvstoragefile.h"

namespace NGJServer {

    //////////////////////////////////////////////////////////////////
    bool TClientConfig::LoadFile(
        std::shared_ptr<TLog> log,
        std::string_view path,
        std::vector<TCommandPair>& comands) {
        auto m_log = log;
        try {
            std::ifstream input(path.data());
            if (!input.is_open()) {
                GJS_FATAL(std::move(std::string("failed open commands file: ").append(path)));
                throw std::exception();
            }
            for(std::string line; getline(input, line);) {
                if (line.size()) {
                    TCommandPair command = TCommandPair::Parse(line);
                    comands.emplace_back(std::move(command));
                }
            }
        }
        catch (std::exception& exception) {
            std::string report;
            report.append("CMD file load failed. Filepath: ").append(path);
            report.append(" Error: ").append(exception.what());
            GJS_FATAL(std::move(report));
            throw;
        }

        return true;
    }

    //--------------------------------------------------------------//
    bool TClientConfig::StoreFile(
        std::shared_ptr<TLog> log,
        std::string_view path,
        const std::vector<TCommandPair>& comands) {
        auto m_log = log;
        std::ofstream file;
        try {
            file.open(path.data(), std::ios_base::out & std::ios_base::binary);
            if (!file.is_open()) {
                GJS_ERROR(std::move(std::string("CMD file create failed. Path: ").append(path)));
                return false;
            }
        
            for (const auto& item: comands) {
                file << item.ToString() << std::endl;
            }
        }
        catch (const std::exception& except) {
            GJS_ERROR(std::move(std::string("KV file write failed.").append(except.what())));
            return false;
        }

        return true;
    }

    //////////////////////////////////////////////////////////////////
    bool TClientConfig::TCommandPair::operator==(const TCommandPair& other) const {
        return m_cmd == other.m_cmd && m_args == other.m_args;
    }

    //--------------------------------------------------------------//
    std::string TClientConfig::TCommandPair::ToString() const {
        std::string res;
        res.reserve(6 + m_args.size());
        switch (m_cmd)
        {
        case ECMD::READ_REQUEST:
            res.append("get ");
            break;
        case ECMD::WRITE_REQUEST:
            res.append("set ");
            break;
        default:
            throw std::runtime_error("Invalid cmd");
        }
        res.append(m_args);
        return res;
    }

    //--------------------------------------------------------------//
    TClientConfig::TCommandPair TClientConfig::TCommandPair::Parse(std::string_view line) {
        TCommandPair cmd_pair;
        cmd_pair.m_cmd = ECMD::None;

        std::string_view command(line.data(), (std::min)(line.size(), (size_t)3));
        if (!command.compare("get")) {
            cmd_pair.m_cmd = ECMD::READ_REQUEST;
        }
        else if (!command.compare("set")) {
            cmd_pair.m_cmd = ECMD::WRITE_REQUEST;
        }
        else {
            return cmd_pair;
        }

        std::string cmd_arg(line.data() + command.size() + 1, line.size() - command.size() - 1);
        cmd_pair.m_args = std::move(cmd_arg);
        return cmd_pair;
    }

    //////////////////////////////////////////////////////////////////
} // namespace NGJServer
