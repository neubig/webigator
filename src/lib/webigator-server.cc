#include <boost/foreach.hpp>

#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <webigator/webigator-server.h>
#include <webigator/data-store.h>

using namespace webigator;
using namespace std;
using namespace boost;
using namespace xmlrpc_c;

typedef std::map<std::string, value> params_t;

// A class to add an example
class UnlabeledExampleAdder : public method
{

public:
    UnlabeledExampleAdder(WebigatorServer & server) : server_(&server) {
        this->_signature = "i:S";
        this->_help = "Adds an example to the cache";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator text_it = params.find("text");
        params_t::const_iterator id_it = params.find("id");
        if (text_it == params.end() || id_it == params.end())
            throw fault("Missing text or id", fault::CODE_PARSE);
        string text = value_string(text_it->second);
        int id = value_int(id_it->second);
        // cerr << "Adding text=" << text << ", id=" << id << endl;

        TextExample exp(id, text);
        exp.SetScore(server_->GetClassifier().GetBinaryScore(exp));
        server_->GetDataStore().AddNewExample(exp);

        // Return 1 on success
        *retvalP = value_int(1);
    }

private:
    WebigatorServer * server_;

};

// A class to add an example
class LabeledExampleAdder : public method
{

public:
    LabeledExampleAdder(WebigatorServer & server, bool keyword) 
                : server_(&server), keyword_(keyword) {
        this->_signature = "i:S";
        this->_help = "Adds a labeled example to the model";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator text_it = params.find("text");
        params_t::const_iterator id_it = params.find("id");
        params_t::const_iterator lab_it = params.find("lab");
        if (text_it == params.end() || id_it == params.end() || lab_it == params.end())
            throw fault("Missing text, id, or lab", fault::CODE_PARSE);
        string text = value_string(text_it->second);
        int id = value_int(id_it->second);
        int lab = value_int(lab_it->second);
        // cerr << "Adding text=" << text << ", id=" << id << endl;

        TextExample exp(id, text, lab);
        server_->GetClassifier().UpdateWithLabeledExample(
            exp, keyword_ ? Classifier::KEYWORD : Classifier::UNSPECIFIED);
        server_->GetDataStore().SetFinished(id);

        // Return 1 on success
        *retvalP = value_int(1);
    }

private:
    WebigatorServer * server_;
    bool keyword_;

};

// A class to add an example
class BestPopper : public method
{

public:
    BestPopper(WebigatorServer & server) : server_(&server) {
        this->_signature = "S:";
        this->_help = "Pop the best example from the cache";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        param_list.verifyEnd(0);
        // Get the arguments
        TextExample exp;
        params_t ret; 
        if(server_->GetDataStore().GetCacheSize() != 0) {
            exp = server_->GetDataStore().PopNextExample();
            // Save the arguments
            ret["text"] = value_string(Dict::PrintWords(exp.GetString()));
            ret["id"] = value_int(exp.GetId());
            ret["lab"] = value_int(exp.GetLabel());
            ret["score"] = value_double(exp.GetScore());
        }

        // Return 1 on success
        *retvalP = value_struct(ret);
    }

private:
    WebigatorServer * server_;

};

// A class to add an example
class WeightGetter : public method
{

public:
    WeightGetter(WebigatorServer & server) : server_(&server) {
        this->_signature = "S:";
        this->_help = "Get model weights";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        param_list.verifyEnd(0);
        // Get the arguments
        TextExample exp;
        params_t ret; 
        BOOST_FOREACH(const SparseMap::value_type & val, server_->GetClassifier().GetBinaryWeights()) {
            const GenericString<int> & fsym = Dict::FeatSym(val.first);
            ret[Dict::PrintWords(fsym)] = value_double(val.second);
        }

        // Return 1 on success
        *retvalP = value_struct(ret);
    }

private:
    WebigatorServer * server_;

};

// Run the model
void WebigatorServer::Run(const ConfigWebigatorServer & config) {

    registry my_registry;

    methodPtr uae(new UnlabeledExampleAdder(*this));
    my_registry.addMethod("add_unlabeled", uae);

    methodPtr lea(new LabeledExampleAdder(*this, false));
    my_registry.addMethod("add_labeled", lea);

    methodPtr ka(new LabeledExampleAdder(*this, true));
    my_registry.addMethod("add_keyword", ka);

    methodPtr wg(new WeightGetter(*this));
    my_registry.addMethod("get_weights", wg);

    methodPtr bp(new BestPopper(*this));
    my_registry.addMethod("pop_best", bp);

    abyss_server_.reset(new serverAbyss(
        serverAbyss::constrOpt()
        .registryP(&my_registry)
        .portNumber(config.GetInt("port"))
    ));
    serverAbyss::shutdown shutdown(abyss_server_.get());
    my_registry.setShutdown(&shutdown);

    abyss_server_->run();
}
