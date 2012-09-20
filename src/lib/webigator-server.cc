#include <boost/foreach.hpp>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <webigator/webigator-server.h>
#include <webigator/data-store.h>

using namespace webigator;
using namespace std;
using namespace boost;

typedef std::map<std::string, xmlrpc_c::value> params_t;

class ParamUtils {

public:
    static string GetMandatoryArgument(const params_t & params, const string & label) {
        params_t::const_iterator si = params.find(label);
        if (si == params.end()) {
            ostringstream oss; oss << "Missing mandatory argument " << label;
            throw xmlrpc_c::fault(oss.str(), xmlrpc_c::fault::CODE_PARSE);
        }
        return string((xmlrpc_c::value_string(si->second)));
    }

    static string GetOptionalArgument(const params_t & params, const string & label) {
        params_t::const_iterator si = params.find(label);
        return (si == params.end()) ? "" : string((xmlrpc_c::value_string(si->second)));
    }
    
    static bool GetBooleanArgument(const params_t & params, const string & label) {
        return params.find(label) != params.end(); 
    }

};

// A class to add an example
class UnlabeledExampleAdder : public xmlrpc_c::method
{

public:
    UnlabeledExampleAdder(WebigatorServer & server) :
            server_(&server) {
        // signature and help strings are documentation -- the client
        // can query this information with a system.methodSignature and
        // system.methodHelp RPC.
        this->_signature = "S:S";
        this->_help = "Adds an example to the cache";
    }

    void execute(xmlrpc_c::paramList const& param_list, xmlrpc_c::value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        string text = ParamUtils::GetMandatoryArgument(params, "text"), 
               id = ParamUtils::GetMandatoryArgument(params, "id");

        TextExample exp(atoi(id.c_str()), text);
        exp.SetScore(server_->GetClassifier().GetBinaryScore(exp));
        server_->GetDataStore().AddNewExample(exp);

    }

private:
    WebigatorServer * server_;

};

// Run the model
void WebigatorServer::Run(const ConfigWebigatorServer & config) {

    xmlrpc_c::registry my_registry;

    xmlrpc_c::methodPtr uae(new UnlabeledExampleAdder(*this));

    my_registry.addMethod("add_unlab_examp", uae);

    xmlrpc_c::serverAbyss my_abyss_server(
        my_registry,
        config.GetInt("port"),
        config.GetString("log")
    );

    my_abyss_server.run();
    THROW_ERROR("Webigator server should never return");
}
