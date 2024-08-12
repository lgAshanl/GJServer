#include "gjserver/server/gjserver.h"

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: gjserver <port>\n";
        return 1;
    }

    NGJServer::TGJServer server(std::atoi(argv[1]), "kvfile.txt");
    server.Start();
    while (true)
        std::this_thread::sleep_for(
            std::chrono::microseconds(NGJServer::TDuration::Seconds(10)));

    return 0;
}