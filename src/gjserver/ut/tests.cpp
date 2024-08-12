#include <gtest/gtest.h>

#include "gjserver/client/gjclient.h"
#include "gjserver/server/gjserver.h"

namespace NGJServerTest {

//////////////////////////////////////////////////////////////////

    class TGJServerTest : public ::testing::Test {
    public:
        using value_t = uint64_t;

        TGJServerTest() {
            std::filesystem::remove(m_serverKVFilePath);
        }

        TGJServerTest(const TGJServerTest& other) = delete;
        TGJServerTest(TGJServerTest&& other) noexcept = delete;
        TGJServerTest& operator=(const TGJServerTest& other) = delete;
        TGJServerTest& operator=(TGJServerTest&& other) noexcept = delete;

        virtual void SetUp() {
            ASSERT_TRUE(CreateFile(m_serverKVFilePath));
        }

        virtual void TearDown() {
            ASSERT_TRUE(std::filesystem::remove(m_serverKVFilePath));
            for (const auto& item : m_clientCMDFiles) {
                std::filesystem::remove(item);
            }
        }

        static bool bump(std::shared_ptr<NGJServer::TServerSession> session, const NGJServer::THeader* request) {
            (void)session;
            (void)request;
            return false;
        }

        ///*
        static bool bumpClient(std::shared_ptr<NGJServer::TClientSession> session, const NGJServer::THeader* request) {
            (void)session;
            (void)request;
            return false;
        }
        //*/

        static bool disk(std::shared_ptr<NGJServer::TClientSession> session) {
            (void)session;
            return false;
        }

        static bool CreateFile(std::string_view path) {
            if (std::filesystem::exists(path.data()))
                return false;

            std::ofstream file(path.data());
            file.close();
            return true;
        }

    public:
        static constexpr std::string_view m_serverPort = "30666";

        static constexpr std::string_view m_serverKVFilePath = "./kvstorage.txt";

        static constexpr std::string_view m_clientConnectAddr = "127.0.0.1";

        static constexpr uint32_t m_maxClients = 8;

        static constexpr std::array<std::string_view, m_maxClients> m_clientCMDFiles = {
            "./ops40666.txt",
            "./ops41666.txt",
            "./ops42666.txt",
            "./ops43666.txt",
            "./ops44666.txt",
            "./ops45666.txt",
            "./ops46666.txt",
            "./ops47666.txt",
        };
    };

    //////////////////////////////////////////////////////////////////
    TEST_F(TGJServerTest, ServerInit0) {
        std::shared_ptr<NGJServer::TLog> log(new NGJServer::TLog());
        NGJServer::TServer h(log, bump, 0);
        NGJServer::TGJServer as(30666, "CMakeCache.txt");

        NGJServer::TClient sd(log, bumpClient, disk, std::to_string(0), std::to_string(0));
        NGJServer::TGJClient gds(std::to_string(0), std::to_string(0));
    }

    //////////////////////////////////////////////////////////////////
    TEST_F(TGJServerTest, ServerInitEmpty) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());
        server.Start();
        server.Stop();
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ServerInitNotEmpty) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());

        const std::unordered_map<std::string, std::string> map {
            {"1", "one"},
            {"2", "two"},
            {"3", "three"},
            {"4", "four"},
            {"5", "five"},
        };

        NGJServer::TKVStorageFile::StoreFile(server.m_log, m_serverKVFilePath.data(), map);
        server.Start();
        server.Stop();

        std::unordered_map<std::string, std::string> result_map;
        NGJServer::TKVStorageFile::LoadFile(server.m_log, m_serverKVFilePath.data(), result_map);

        ASSERT_EQ(map, result_map);
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ClientInit) {
        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ClientTestConnect) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());
        server.Start();

        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();
        ASSERT_TRUE(client.TestConnect());
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ClientCheckCmdFile) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());
        server.Start();

        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();
        const std::vector<NGJServer::TClientConfig::TCommandPair> ops {
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::WRITE_REQUEST, "1 new_one" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::READ_REQUEST, "2 new_two" },
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::READ_REQUEST, "3" }
        };
        NGJServer::TClientConfig::StoreFile(client.m_log, m_clientCMDFiles[0], ops);
        client.ProcessFile(m_clientCMDFiles[0].data());

        client.Stop();
        server.Stop();

        std::vector<NGJServer::TClientConfig::TCommandPair> result_ops;
        NGJServer::TClientConfig::LoadFile(client.m_log, m_clientCMDFiles[0], result_ops);

        ASSERT_EQ(ops, result_ops);
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ClientProcessFile) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());

        const std::unordered_map<std::string, std::string> map {
            {"1", "one"},
            {"2", "two"},
            {"3", "three"},
            {"4", "four"},
            {"5", "five"},
        };

        NGJServer::TKVStorageFile::StoreFile(server.m_log, m_serverKVFilePath.data(), map);
        server.Start();

        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();

        const std::vector<NGJServer::TClientConfig::TCommandPair> ops {
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::WRITE_REQUEST, "1 new_one" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::WRITE_REQUEST, "2 new_two" },
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::READ_REQUEST, "3" }
        };
        NGJServer::TClientConfig::StoreFile(client.m_log, m_clientCMDFiles[0], ops);
        client.ProcessFile(m_clientCMDFiles[0].data());

        client.Stop();
        server.Stop();

        std::unordered_map<std::string, std::string> result_map;
        NGJServer::TKVStorageFile::LoadFile(server.m_log, m_serverKVFilePath.data(), result_map);

        const std::unordered_map<std::string, std::string> expected_map {
            {"1", "new_one"},
            {"2", "new_two"},
            {"3", "three"},
            {"4", "four"},
            {"5", "five"},
        };

        ASSERT_EQ(expected_map, result_map);
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, SeveralClientsProcessFile) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());

        const std::unordered_map<std::string, std::string> map {
            {"0", "zero"},
            {"1", "one"},
            {"2", "two"},
            {"3", "three"},
            {"4", "four"},
        };

        NGJServer::TKVStorageFile::StoreFile(server.m_log, m_serverKVFilePath.data(), map);
        server.Start();

        constexpr uint32_t NCLIENTS = 4;
        std::list<NGJServer::TGJClient> clients;
        for (uint32_t i = 0; i < NCLIENTS; ++i) {
            clients.emplace_back(m_clientConnectAddr.data(), m_serverPort.data());
            clients.back().Start();

            const std::vector<NGJServer::TClientConfig::TCommandPair> ops {
                { NGJServer::ECMD::READ_REQUEST, std::to_string(i) },
                { NGJServer::ECMD::WRITE_REQUEST, std::to_string(i).append(" 2")},
                { NGJServer::ECMD::READ_REQUEST, std::to_string(i) },
                { NGJServer::ECMD::WRITE_REQUEST, std::to_string(i).append(" 6") }
            };
            NGJServer::TClientConfig::StoreFile(clients.back().m_log, m_clientCMDFiles[i], ops);
        }

        std::list<std::thread> client_procs;
        auto client_iter = clients.begin();
        for (uint32_t i = 0; i < NCLIENTS; ++i, ++client_iter) {
            client_procs.emplace_back(
                [&client = *client_iter, i]() {
                    client.ProcessFile(m_clientCMDFiles[i].data());
                    client.Stop();
                });
        }

        for (uint32_t i = 0; i < NCLIENTS; ++i) {
            client_procs.back().join();
            client_procs.pop_back();
        }

        server.Stop();

        std::unordered_map<std::string, std::string> result_map;
        NGJServer::TKVStorageFile::LoadFile(server.m_log, m_serverKVFilePath.data(), result_map);

        const std::unordered_map<std::string, std::string> expected_map {
            {"0", "6"},
            {"1", "6"},
            {"2", "6"},
            {"3", "6"},
            {"4", "four"},
        };

        ASSERT_EQ(expected_map, result_map);
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, ClientReconnect) {
        std::unique_ptr<NGJServer::TGJServer> server(
            new NGJServer::TGJServer(30666, m_serverKVFilePath.data()));

        const std::unordered_map<std::string, std::string> map {
            {"1", "one"},
            {"2", "two"},
            {"3", "three"},
            {"4", "four"},
            {"6", "six"},
        };

        NGJServer::TKVStorageFile::StoreFile(server->m_log, m_serverKVFilePath.data(), map);
        server->Start();

        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();

        const std::vector<NGJServer::TClientConfig::TCommandPair> ops {
            { NGJServer::ECMD::WRITE_REQUEST, "1 new_one" },
            { NGJServer::ECMD::WRITE_REQUEST, "2 new_two" },
            { NGJServer::ECMD::WRITE_REQUEST, "3 new_three" },
            { NGJServer::ECMD::WRITE_REQUEST, "4 new_four" },
            { NGJServer::ECMD::WRITE_REQUEST, "5 new_five" },
        };
        NGJServer::TClientConfig::StoreFile(client.m_log, m_clientCMDFiles[0], ops);

        std::thread client_thread(
            [&client]() {
                client.ProcessFileSlow(m_clientCMDFiles[0].data());
                client.Stop();
            });

        server->Stop();
        server.reset(nullptr);

        server.reset(new NGJServer::TGJServer(30666, m_serverKVFilePath.data()));
        server->Start();

        client_thread.join();
        server->Stop();

        std::unordered_map<std::string, std::string> result_map;
        NGJServer::TKVStorageFile::LoadFile(server->m_log, m_serverKVFilePath.data(), result_map);

        const std::unordered_map<std::string, std::string> expected_map {
            {"1", "new_one"},
            {"2", "new_two"},
            {"3", "new_three"},
            {"4", "new_four"},
            {"5", "new_five"},
            {"6", "six"}
        };

        ASSERT_EQ(expected_map, result_map);
    }

    //--------------------------------------------------------------//
    TEST_F(TGJServerTest, CheckTotalReport) {
        NGJServer::TGJServer server(30666, m_serverKVFilePath.data());
        server.Start();

        NGJServer::TGJClient client(m_clientConnectAddr.data(), m_serverPort.data());
        client.Start();
        const std::vector<NGJServer::TClientConfig::TCommandPair> ops {
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::WRITE_REQUEST, "1 new_one" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::READ_REQUEST, "2 new_two" },
            { NGJServer::ECMD::READ_REQUEST, "1" },
            { NGJServer::ECMD::READ_REQUEST, "2" },
            { NGJServer::ECMD::READ_REQUEST, "3" }
        };
        NGJServer::TClientConfig::StoreFile(client.m_log, m_clientCMDFiles[0], ops);
        std::this_thread::sleep_for(std::chrono::microseconds(NGJServer::TDuration::MilliSeconds(500)));
        client.ProcessFile(m_clientCMDFiles[0].data());

        std::this_thread::sleep_for(std::chrono::microseconds(NGJServer::TStat::DEFAULT_FRAME_DURATION));
        std::this_thread::sleep_for(std::chrono::microseconds(NGJServer::TStat::DEFAULT_FRAME_DURATION));
        std::this_thread::sleep_for(std::chrono::microseconds(NGJServer::TDuration::MilliSeconds(500)));

        client.Stop();
        server.Stop();

        std::vector<NGJServer::TClientConfig::TCommandPair> result_ops;
        NGJServer::TClientConfig::LoadFile(client.m_log, m_clientCMDFiles[0], result_ops);

        ASSERT_EQ(ops, result_ops);
    }

} // NGJServerTest