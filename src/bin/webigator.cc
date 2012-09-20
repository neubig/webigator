#include <webigator/config-webigator-runner.h>
#include <webigator/webigator-runner.h>

using namespace webigator;
using namespace std;

int main(int argc, char** argv) {
    // load the arguments
    ConfigWebigatorRunner conf;
    vector<string> args = conf.loadConfig(argc,argv);
    // train the reorderer
    WebigatorRunner runner;
    runner.Run(conf);
}
