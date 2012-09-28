#include <boost/foreach.hpp>

#include <xmlrpc-c/xml.hpp>
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
        if (text_it == params.end() || id_it == params.end()) {

            throw fault("Missing text or id", fault::CODE_PARSE);
        }
        string text = value_string(text_it->second);
        long long id;
        if(id_it->second.type() == value::TYPE_STRING) {
            istringstream iss(value_string(id_it->second));
            iss >> id;
        } else {
            id = value_int(id_it->second);
        }

        TextExample exp(id, text);
        exp.SetScore(server_->GetClassifier().GetBinaryMargin(exp, TextClassifier::GEOM_MEAN));
        PRINT_DEBUG("Adding unlabeled: text="<<text<< ", id=" << id << ", score=" << exp.GetScore() << endl, 1);
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
        long long id;
        if(id_it->second.type() == value::TYPE_STRING) {
            istringstream iss(value_string(id_it->second));
            iss >> id;
        } else {
            id = value_int(id_it->second);
        }
        int lab = value_int(lab_it->second);
        PRINT_DEBUG("Adding "<<(keyword_?"keyword":"labeled")<<": text="<<text<< ", id=" << id << ", lab=" << lab << endl, 1);
        
        // Add the keyword to the data store
        if(keyword_)
            server_->GetDataStore().AddKeyword(text);
            
        // Add the example
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
            exp = server_->GetDataStore().PopNextExample(server_->GetClassifier());
            // Save the arguments
            string text = Dict::PrintWords(exp.GetString());
            ret["text"] = value_string(text);
            ostringstream oss; oss << exp.GetId();
            ret["id"] = value_string(oss.str());
            ret["lab"] = value_int(exp.GetLabel());
            ret["score"] = value_double(exp.GetScore());
            PRINT_DEBUG("Popping best: text="<<text<< ", id=" << exp.GetId() << ", lab=" << exp.GetLabel() << ", score=" << exp.GetScore() << endl, 1);
        } else {
            PRINT_DEBUG("Popping best, but none exists" << endl, 1);
        }
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
        this->_signature = "S:i";
        this->_help = "Get model weights";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        int id = param_list.getInt(0);
        param_list.verifyEnd(1);
        // Get the arguments
        TextExample exp;
        params_t ret; 
        BOOST_FOREACH(const SparseMap::value_type & val, server_->GetClassifier().GetWeights(id)) {
            const GenericString<int> & fsym = Dict::FeatSym(val.first);
            ret[Dict::PrintWords(fsym)] = value_double(val.second);
        }

        // Return 1 on success
        *retvalP = value_struct(ret);
    }

private:
    WebigatorServer * server_;

};

// A class to add an example
class KeywordGetter : public method
{

public:
    KeywordGetter(WebigatorServer & server) : server_(&server) {
        this->_signature = "A:";
        this->_help = "Get keywords";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        param_list.verifyEnd(0);
        // Get the arguments
        vector<value> ret;
        BOOST_FOREACH(const string & str, server_->GetDataStore().GetKeywords()) {
            PRINT_DEBUG("Returning keyword " << str << endl, 1);
            ret.push_back(value_string(str));
        }

        // Return 1 on success
        *retvalP = value_array(ret);
    }

private:
    WebigatorServer * server_;

};

// A class to add an example
class CacheRescorer : public method
{

public:
    CacheRescorer(WebigatorServer & server) : server_(&server) {
        this->_signature = "i:";
        this->_help = "Rescore the values in the cache";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        PRINT_DEBUG("Rescoring cache" << endl, 1);

        server_->GetDataStore().RescoreCache(server_->GetClassifier());

        // Return the size of the cache on success
        *retvalP = value_int(server_->GetDataStore().GetCache().size());

        PRINT_DEBUG("Finished rescoring cache" << endl, 1);
    }

private:
    WebigatorServer * server_;

};

// Run the model
void WebigatorServer::Run(const ConfigWebigatorServer & config) {

    GlobalVars::debug = config.GetInt("debug");

    if(config.GetString("learner") == "nb")
        classifier_ = TextClassifier(2, config.GetInt("feature_n"), Classifier::NAIVE_BAYES);
    else if(config.GetString("learner") == "perceptron")
        classifier_ = TextClassifier(2, config.GetInt("feature_n"), Classifier::PERCEPTRON);
    else
        THROW_ERROR("Bad -learner option: " << config.GetString("learner"));

    registry my_registry;

    methodPtr uae(new UnlabeledExampleAdder(*this));
    my_registry.addMethod("add_unlabeled", uae);

    methodPtr lea(new LabeledExampleAdder(*this, false));
    my_registry.addMethod("add_labeled", lea);

    methodPtr ka(new LabeledExampleAdder(*this, true));
    my_registry.addMethod("add_keyword", ka);

    methodPtr wg(new WeightGetter(*this));
    my_registry.addMethod("get_weights", wg);

    methodPtr wk(new KeywordGetter(*this));
    my_registry.addMethod("get_keywords", wk);

    methodPtr bp(new BestPopper(*this));
    my_registry.addMethod("pop_best", bp);

    methodPtr rescore(new CacheRescorer(*this));
    my_registry.addMethod("rescore", rescore);

    abyss_server_.reset(new serverAbyss(
        serverAbyss::constrOpt()
        .registryP(&my_registry)
        .portNumber(config.GetInt("port"))
    ));
    serverAbyss::shutdown shutdown(abyss_server_.get());
    my_registry.setShutdown(&shutdown);

    abyss_server_->run();
}
