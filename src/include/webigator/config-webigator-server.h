#ifndef CONFIG_WEBIGATOR_SERVER_H__
#define CONFIG_WEBIGATOR_SERVER_H__

#include <string>
#include <vector>
#include <cstdlib>
#include <sstream>
#include <webigator/util.h>
#include <webigator/config-base.h>

namespace webigator {

class ConfigWebigatorServer : public ConfigBase {

public:

    ConfigWebigatorServer() : ConfigBase() {
        minArgs_ = 0;
        maxArgs_ = 0;

        SetUsage(
"~~~ webigator ~~~\n"
"  by Graham Neubig\n"
"\n"
"A server for aggregation and scoring of text data.\n"
"  Usage: webigator\n"
);

        AddConfigEntry("port", "9597", "Where to write log files");
        AddConfigEntry("debug", "0", "Level of debugging info to print");
        AddConfigEntry("learner", "nb", "The learner to use");
        AddConfigEntry("feature_n", "2", "The length of the feature vector");
        AddConfigEntry("uniq", "true", "Do not return duplicate values");
        AddConfigEntry("cache", "-1", "Cache size (-1 == unlimited)");

    }
	
};

}

#endif
