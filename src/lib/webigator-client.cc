#include <boost/foreach.hpp>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/client_simple.hpp>

#include <webigator/webigator-client.h>
#include <webigator/data-store.h>

using namespace webigator;
using namespace std;
using namespace boost;
using namespace xmlrpc_c;

typedef std::map<std::string, value> params_t;

// Run the model
void WebigatorClient::Run(const ConfigWebigatorClient & config) {

    const string server_url(config.GetString("server"));
    cerr << server_url << endl;

    clientSimple my_client;
    value result;
    params_t params;
    params["id"] = value_int(50);
    params["text"] = value_string("this is a test");
    xmlrpc_c::paramList myParamList;
    myParamList.add(value_struct(params));

    my_client.call(server_url, "addunlab", myParamList, &result);
    // my_client.call(server_url, method_name, "", &result);
    int ret = value_int(result);
    cerr << "Returned " << ret << endl;

}
