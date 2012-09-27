#ifndef WEBIGATOR_SERVER_H__ 
#define WEBIGATOR_SERVER_H__

#include <iostream>
#include <fstream>
#include <webigator/config-webigator-server.h>
#include <webigator/data-store.h>
#include <webigator/text-classifier.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <boost/shared_ptr.hpp>
#include <tr1/unordered_map>

namespace webigator {

// A class to build features for the filterer
class WebigatorServer {
public:

    WebigatorServer() { }
    ~WebigatorServer() { }
    
    // Run the model
    void Run(const ConfigWebigatorServer & config);

    const DataStore & GetDataStore() const { return data_store_; }
    DataStore & GetDataStore() { return data_store_; }
    const TextClassifier & GetClassifier() const { return classifier_; }
    TextClassifier & GetClassifier() { return classifier_; }
    const boost::shared_ptr<xmlrpc_c::serverAbyss> & GetAbyssServer() const { return abyss_server_; }
    boost::shared_ptr<xmlrpc_c::serverAbyss> & GetAbyssServer() { return abyss_server_; }

private:

    boost::shared_ptr<xmlrpc_c::serverAbyss> abyss_server_;
    DataStore data_store_;
    TextClassifier classifier_;

};

}

#endif

