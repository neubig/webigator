#include <webigator/config-webigator-server.h>
#include <webigator/webigator-server.h>

using namespace webigator;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigWebigatorServer conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    WebigatorServer server;
    server.Run(conf);
}
