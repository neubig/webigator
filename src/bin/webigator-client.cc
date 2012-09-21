#include <webigator/config-webigator-client.h>
#include <webigator/webigator-client.h>

using namespace webigator;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigWebigatorClient conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    WebigatorClient client;
    client.Run(conf);
}
