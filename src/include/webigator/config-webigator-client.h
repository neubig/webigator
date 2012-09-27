#ifndef CONFIG_WEBIGATOR_SERVER_H__
#define CONFIG_WEBIGATOR_SERVER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <webigator/util.h>
#include <webigator/config-base.h>

namespace webigator {

class ConfigWebigatorClient : public ConfigBase {

public:

    ConfigWebigatorClient() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ webigator-client ~~~\n"
"  by Graham Neubig\n"
"\n"
"A client for aggregation and scoring of text data.\n"
"  Usage: webigator-client\n"
);

        AddConfigEntry("server", "http://localhost:9597/RPC2", "The server to connect to");

    }
	
};

}

#endif
