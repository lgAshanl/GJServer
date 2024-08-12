#include "gjserver/client/gjclient.h"

int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Usage: gjclient <addr> <port> <file> \n";
        return 1;
    }

    NGJServer::TGJClient client(argv[1], argv[2]);
    client.Start();

    client.ProcessFile(argv[3]);

    return 0;
}