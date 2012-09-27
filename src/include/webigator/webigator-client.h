#ifndef WEBIGATOR_CLIENT_H__ 
#define WEBIGATOR_CLIENT_H__

#include <iostream>
#include <fstream>
#include <webigator/config-webigator-client.h>
#include <tr1/unordered_map>

namespace webigator {

// A class to build features for the filterer
class WebigatorClient {
public:

    WebigatorClient() { }
    ~WebigatorClient() { }
    
    // Run the model
    void Run(const ConfigWebigatorClient & config);

private:

};

}

#endif

