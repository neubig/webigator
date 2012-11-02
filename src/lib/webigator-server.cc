#include <boost/foreach.hpp>

#include <xmlrpc-c/xml.hpp>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>

#include <tr1/unordered_set>
#include <cmath>

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
            throw fault("Missing text, or id in UnlabeledExampleAdder", fault::CODE_PARSE);
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
        typedef std::pair<int,int> IntPair;
        BOOST_FOREACH(IntPair val, server_->GetIdMap()) {
            exp.SetScore(server_->GetClassifier(val.first).GetBinaryMargin(exp));
            if(isnan(exp.GetScore()))
                THROW_ERROR("Score is NAN @ " << id << ": " << text);
            PRINT_DEBUG("Adding unlabeled: task="<<val.first<<", text="<<text<< ", id=" << id << ", score=" << exp.GetScore() << endl, 2);
            server_->GetDataStore(val.first).AddNewExample(exp);
        }

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
        params_t::const_iterator task_it = params.find("task_id");
        params_t::const_iterator text_it = params.find("text");
        params_t::const_iterator id_it = params.find("id");
        params_t::const_iterator lab_it = params.find("lab");
        if (task_it == params.end() || text_it == params.end() || id_it == params.end() || lab_it == params.end())
            throw fault("Missing task_id, text, id, or lab in LabeledExampleAdder", fault::CODE_PARSE);
        string text = value_string(text_it->second);
        long long id;
        if(id_it->second.type() == value::TYPE_STRING) {
            istringstream iss(value_string(id_it->second));
            iss >> id;
        } else {
            id = value_int(id_it->second);
        }
        int lab = value_int(lab_it->second);
        int task = value_int(task_it->second);
        PRINT_DEBUG("Adding "<<(keyword_?"keyword":"labeled")<<": task="<<task<<", text="<<text<< ", id=" << id << ", lab=" << lab << endl, 1);
        
        // Add the keyword to the data store
        if(keyword_)
            server_->GetDataStore(task).AddKeyword(text);
            
        // Add the example
        TextExample exp(id, text, lab);
        server_->GetClassifier(task).UpdateWithLabeledExample(
            exp, keyword_ ? Classifier::KEYWORD : Classifier::UNSPECIFIED);
        server_->GetDataStore(task).SetFinished(id);

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
        this->_signature = "S:S";
        this->_help = "Pop the best example from the cache";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator task_it = params.find("task_id");
        if (task_it == params.end())
            throw fault("Missing task_id in BestPopper", fault::CODE_PARSE);
        int task = value_int(task_it->second);

        // Get the arguments
        TextExample exp;
        params_t ret; 
        if(server_->GetDataStore(task).GetCacheSize() != 0) {
            exp = server_->GetDataStore(task).PopNextExample(server_->GetClassifier(task));
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
        this->_signature = "S:S";
        this->_help = "Get model weights";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator task_it = params.find("task_id");
        params_t::const_iterator id_it = params.find("id");
        if (task_it == params.end() || id_it == params.end())
            throw fault("Missing task_id or id in WeightGetter", fault::CODE_PARSE);
        int id = value_int(id_it->second);
        int task = value_int(task_it->second);


        // Get the arguments
        TextExample exp;
        params_t ret; 
        BOOST_FOREACH(const SparseMap::value_type & val, server_->GetClassifier(task).GetWeights(id)) {
            const GenericString<int> & fsym = Dict::FeatSym(val.first);
            ret[Dict::PrintWords(fsym)] = value_double(val.second);
        }

        // Return 1 on success
        *retvalP = value_struct(ret);
    }

private:
    WebigatorServer * server_;

};

// Get keywords
class KeywordGetter : public method
{

public:
    KeywordGetter(WebigatorServer & server) : server_(&server) {
        this->_signature = "A:S";
        this->_help = "Get keywords";
    }

    void execute(paramList const& param_list, value * const retvalP) {

        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator task_it = params.find("task_id");
        if (task_it == params.end())
            throw fault("Missing task_id in KeywordGetter", fault::CODE_PARSE);
        int task = value_int(task_it->second);

        // Get the keywords for a single task
        std::tr1::unordered_set<string> keywords;
        if(task == -1) {
            BOOST_FOREACH(const shared_ptr<Task> & task, server_->GetTasks())
                BOOST_FOREACH(const string & key, task->GetDataStore().GetKeywords())
                    keywords.insert(key);
        } else {
            BOOST_FOREACH(const string & key, server_->GetDataStore(task).GetKeywords())
                keywords.insert(key); 
        }

        vector<value> ret;
        BOOST_FOREACH(const string & key, keywords) {
            PRINT_DEBUG("Returning keyword " << key << endl, 1);
            ret.push_back(value_string(key));
        }

        // Return 1 on success
        *retvalP = value_array(ret);
    }

private:
    WebigatorServer * server_;

};

// Get keywords
class TaskGetter : public method
{

public:
    TaskGetter(WebigatorServer & server) : server_(&server) {
        this->_signature = "A:";
        this->_help = "Get tasks";
    }

    // Return an array of tasks consisting of task id, name, and keyword array
    void execute(paramList const& param_list, value * const retvalP) {

        param_list.verifyEnd(0);
        typedef std::pair<int,int> IntPair;
        vector<value> ret;

        // Get all the taks
        std::tr1::unordered_set<string> tasks;
        BOOST_FOREACH(const IntPair & val, server_->GetIdMap()) {
            params_t task_ret;
            const Task & task = server_->GetTask(val.first);
            vector<value> keywords;
            BOOST_FOREACH(const string & key, task.GetDataStore().GetKeywords())
                keywords.push_back(value_string(key)); 
            task_ret["has_pass"] = value_boolean(task.HasUserPass());
            task_ret["keywords"] = value_array(keywords);
            task_ret["task_id"] = value_int(val.first);
            ret.push_back(value_struct(task_ret));
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
        this->_signature = "i:S";
        this->_help = "Rescore the values in the cache";
    }

    void execute(paramList const& param_list, value * const retvalP) {
        
        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
        // Get the arguments
        params_t::const_iterator task_it = params.find("task_id");
        if (task_it == params.end())
            throw fault("Missing task_id in CacheRescorer", fault::CODE_PARSE);
        int task = value_int(task_it->second);

        PRINT_DEBUG("Rescoring cache" << endl, 1);

        server_->GetDataStore(task).RescoreCache(server_->GetClassifier(task));

        // Return the size of the cache on success
        *retvalP = value_int(server_->GetDataStore(task).GetCache().size());

        PRINT_DEBUG("Finished rescoring cache" << endl, 1);
    }

private:
    WebigatorServer * server_;

};

// A class to add an example
class TaskAdder : public method
{

public:
    TaskAdder(WebigatorServer & server, const ConfigWebigatorServer & config) : server_(&server), config_(&config) {
        this->_signature = "i:S";
        this->_help = "Add a new task";
    }

    void execute(paramList const& param_list, value * const retvalP) {
        
        // Get the parameters
        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
 
        // Create the task
        Task * task = new Task;
        if(config_->GetString("learner") == "nb")
            task->SetClassifier(TextClassifier(2, config_->GetInt("feature_n"), Classifier::NAIVE_BAYES));
        else if(config_->GetString("learner") == "perceptron")
            task->SetClassifier(TextClassifier(2, config_->GetInt("feature_n"), Classifier::PERCEPTRON));
        else
            THROW_ERROR("Bad -learner option: " << config_->GetString("learner"));

        // Add the passs if they exist
        params_t::const_iterator user_it = params.find("user_pass");
        if (user_it != params.end()) {
            string user_pass = value_string(user_it->second);
            task->SetUserPass(user_pass);
        }
        params_t::const_iterator admin_it = params.find("admin_pass");
        if (admin_it != params.end()) {
            string admin_pass = value_string(admin_it->second);
            task->SetAdminPass(admin_pass);
        }
        
        task->GetDataStore().SetUniq(config_->GetBool("uniq"));
        task->GetDataStore().SetMaxCacheSize(config_->GetInt("cache"));
        int task_id = server_->AddTask(shared_ptr<Task>(task));
        PRINT_DEBUG("Added new task " << task_id << endl, 1);

        *retvalP = value_int(task_id);
    }

private:
    WebigatorServer * server_;
    const ConfigWebigatorServer * config_;
};

// A class to add an example
class PassChecker : public method
{

public:

    typedef enum { USER, ADMIN } CheckerType;

    PassChecker(WebigatorServer & server, CheckerType type) : server_(&server), type_(type) {
        this->_signature = "i:S";
        this->_help = "Checks a password for either the user or the admin";
    }

    void execute(paramList const& param_list, value * const retvalP) {
        
        // Get the parameters
        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
 
        // Add the passs if they exist
        params_t::const_iterator task_it = params.find("task_id");
        params_t::const_iterator pass_it = params.find("pass");
        if (task_it == params.end() || pass_it == params.end()) {
            throw fault("Missing task_id or pass in PassChecker", fault::CODE_PARSE);
        }
        int task_id = value_int(task_it->second);
        string pass = value_string(pass_it->second);

        const Task & task = server_->GetTask(task_id);
        if(type_ == USER) {
            PRINT_DEBUG("Checking " << pass << " == " << task.GetUserPass() << endl, 2);
            *retvalP = value_int(pass == task.GetUserPass() ? 1 : 0);
        } else if (type_ == ADMIN) {
            *retvalP = value_int(pass == task.GetAdminPass() ? 1 : 0);
        } else {
            throw fault("Bad password type in PassChecker", fault::CODE_PARSE);
        }
    }

private:
    WebigatorServer * server_;
    CheckerType type_;
};

// A class to get the user password given the admin password
class PassGetter : public method
{

public:

    typedef enum { USER, ADMIN } GetterType;

    PassGetter(WebigatorServer & server) : server_(&server) {
        this->_signature = "s:S";
        this->_help = "Gets the user password given the admin password";
    }

    void execute(paramList const& param_list, value * const retvalP) {
        
        // Get the parameters
        const params_t params = param_list.getStruct(0);
        param_list.verifyEnd(1);
 
        // Add the passs if they exist
        params_t::const_iterator task_it = params.find("task_id");
        params_t::const_iterator pass_it = params.find("pass");
        if (task_it == params.end() || pass_it == params.end()) {
            throw fault("Missing task_id or pass in PassGetter", fault::CODE_PARSE);
        }
        int task_id = value_int(task_it->second);
        string pass = value_string(pass_it->second);
        const Task & task = server_->GetTask(task_id);

        if(pass != task.GetAdminPass())
            throw fault("Bad admin password", fault::CODE_PARSE);
        
        *retvalP = value_string(task.GetUserPass());
    }

private:
    WebigatorServer * server_;
};

// Run the model
void WebigatorServer::Run(const ConfigWebigatorServer & config) {

    GlobalVars::debug = config.GetInt("debug");

    registry my_registry;

    methodPtr uae(new UnlabeledExampleAdder(*this));
    my_registry.addMethod("add_unlabeled", uae);

    methodPtr lea(new LabeledExampleAdder(*this, false));
    my_registry.addMethod("add_labeled", lea);

    methodPtr ka(new LabeledExampleAdder(*this, true));
    my_registry.addMethod("add_keyword", ka);

    methodPtr ta(new TaskAdder(*this, config));
    my_registry.addMethod("add_task", ta);

    methodPtr wg(new WeightGetter(*this));
    my_registry.addMethod("get_weights", wg);

    methodPtr gk(new KeywordGetter(*this));
    my_registry.addMethod("get_keywords", gk);

    methodPtr gt(new TaskGetter(*this));
    my_registry.addMethod("get_tasks", gt);

    methodPtr bp(new BestPopper(*this));
    my_registry.addMethod("pop_best", bp);

    methodPtr rescore(new CacheRescorer(*this));
    my_registry.addMethod("rescore", rescore);
    
    methodPtr check_user_pass(new PassChecker(*this, PassChecker::USER));
    my_registry.addMethod("check_user_pass", check_user_pass);

    methodPtr get_user_pass(new PassGetter(*this));
    my_registry.addMethod("get_user_pass", get_user_pass);

    methodPtr check_admin_pass(new PassChecker(*this, PassChecker::ADMIN));
    my_registry.addMethod("check_admin_pass", check_admin_pass);

    abyss_server_.reset(new serverAbyss(
        serverAbyss::constrOpt()
        .registryP(&my_registry)
        .portNumber(config.GetInt("port"))
    ));
    serverAbyss::shutdown shutdown(abyss_server_.get());
    my_registry.setShutdown(&shutdown);

    abyss_server_->run();
}
