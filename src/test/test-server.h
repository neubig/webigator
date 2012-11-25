#ifndef TEST_SERVER_H__
#define TEST_SERVER_H__

#include "test-base.h"
#include <boost/shared_ptr.hpp>
#include <webigator/webigator-server.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/client_simple.hpp>
#include <xmlrpc-c/xml.hpp>
#include <pthread.h>

using namespace boost;
using namespace xmlrpc_c;

namespace webigator {

typedef std::map<std::string, value> params_t;
typedef std::vector<value> array_t;

value CallServer(const string & server_url, const string & call, const params_t & params) {
        clientSimple my_client;
        value result;
        paramList myParamList;
        myParamList.add(value_struct(params));
        my_client.call(server_url, call, myParamList, &result);
        return result;
}

string RandomString() {
    ostringstream oss;
    for(int i = 0; i < 10; i++) {
        oss << 'a'+rand()%26 << ' ';
    }
    return oss.str();
}

// sample all the values in the batch
void* ThrashServer(void* ptr) {
    int task = *(int*)ptr;
    string server_url = "http://localhost:9601/RPC2";

    for(int i = 0; i < 1000; i++) {
        // Add labeled values
        {
            params_t params;
            params["id"] = value_int(0);
            params["text"] = value_string(RandomString());
            params["task_id"] = value_int(task);
            params["lab"] = value_int(1);
            CallServer(server_url, "add_labeled", params);
        }
        
        // Add unlabeled values
        {
            params_t params;
            params["id"] = value_int(1);
            params["task_id"] = value_int(task);
            params["text"] = value_string(RandomString());
            CallServer(server_url, "add_unlabeled", params);
        }
        {
            params_t params;
            params["id"] = value_int(2);
            params["task_id"] = value_int(task);
            params["text"] = value_string(RandomString());
            CallServer(server_url, "add_unlabeled", params);
        }

        // Check to make sure that we get the best scored one with the proper
        // values
        params_t params_ret;
        {
            params_t params;
            params["task_id"] = value_int(task);
            params_ret = value_struct(CallServer(server_url,"pop_best",params));
        }
    }
    return NULL;
}
     
void *run_server_function( void *ptr )
{
    ConfigWebigatorServer * config = (ConfigWebigatorServer*)ptr;
    WebigatorServer server;
    cerr << "Running server" << endl;
    server.Run(*config);
    cerr << "Done running server" << endl;
    return NULL;
}

class TestServer : public TestBase {

public:

    TestServer() { }

    ~TestServer() { }

    shared_ptr<pthread_t> StartServer(const ConfigWebigatorServer & config) {
        shared_ptr<pthread_t> thread(new pthread_t);
        pthread_create( thread.get(), NULL, run_server_function, (void*)&config);
        // Wait for 50 milliseconds for it to start
        usleep(5e4);
        return thread;
    }

    void StopServer(pthread_t & thread, int port) {
        // Send it a command to terminate    
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        clientSimple my_client;
        value result;
        paramList myParamList;
        ostringstream url; url << "http://localhost:" << config.GetString("port") << "/RPC2";
        cerr << "Sending system.shutdown" << endl;
        my_client.call(url.str(), "system.shutdown", myParamList.add(value_string("Done")), &result);
        cerr << "Done sending system.shutdown" << endl;
        // // Wait for it to stop (This is not working!)
        // pthread_join(thread, NULL);
        usleep(5e4);
    }

    int TestStartStop() {
        int port = 9597;
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        config.SetInt("feature_n", 1);
        shared_ptr<pthread_t> thread = StartServer(config);
        StopServer(*thread, port);
        return 1;
    }
    
    string ValToString(const value & val) {
        ostringstream oss;
        if(val.type() == value::TYPE_STRING)
            oss << (string)value_string(val);
        else if (val.type() == value::TYPE_INT)
            oss << value_int(val);
        else if (val.type() == value::TYPE_DOUBLE)
            oss << value_double(val);
        else 
            THROW_ERROR("Bad type " << val.type());
        return oss.str();
    }

    int CheckParamsMap(const params_t & exp, const params_t & act) {
        typedef params_t::value_type MapPair;
        int ok = 1;
        BOOST_FOREACH(MapPair kv, exp) {
            params_t::const_iterator it = act.find(kv.first);
            if(it == act.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<
                            "] ("<<ValToString(kv.second)<<" != NULL)" << endl;
                ok = 0;
            } else if(ValToString(it->second) != ValToString(kv.second)) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<
                            "] (" << ValToString(kv.second) << " != " << 
                            ValToString(it->second) << ")" << endl;
                ok = 0;
            }
        }
        BOOST_FOREACH(MapPair kv, act) {
            params_t::const_iterator it = exp.find(kv.first);
            if(it == exp.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<
                            "] (NULL != "<<ValToString(kv.second)<<")" << endl;
                ok = 0;
            }
        }
        return ok;
    }

    int CheckValueArray(const array_t & exp, const array_t & act) {
        int ok = 1;
        for(int i = 0; i < (int)max(exp.size(), act.size()); i++) {
            if(i >= (int)exp.size() || 
               i >= (int)act.size() || 
               ValToString(exp[i]) != ValToString(act[i])) {
               
                ok = 0;
                std::cout << "exp["<<i<<"] != act["<<i<<"] (";
                if(i >= (int)exp.size()) std::cout << "NULL";
                else std::cout << ValToString(exp[i]);
                std::cout <<" != ";
                if(i >= (int)act.size()) std::cout << "NULL"; 
                else std::cout << ValToString(act[i]);
                std::cout << ")" << std::endl;
            }
        }
        return ok;
    }

    int GetTask(const string & server_url) {
        clientSimple my_client;
        value result;
        params_t params;
        paramList myParamList;
        myParamList.add(value_struct(params));
        my_client.call(server_url, "add_task", myParamList, &result);
        return value_int(result);
    }

    int TestThrashServer() {
        int port = 9601;
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        config.SetInt("feature_n", 3);
        config.SetString("learner", "perceptron");
        shared_ptr<pthread_t> thread = StartServer(config);
        ostringstream url; url << "http://localhost:" << port << "/RPC2";
        string server_url = url.str();
        
        // Start thrashing the server 10 times
        int task = GetTask(server_url);
        vector<shared_ptr<pthread_t> > threads(4);
        BOOST_FOREACH(shared_ptr<pthread_t> & t, threads) {
            t.reset(new pthread_t);
            pthread_create(t.get(), NULL, ThrashServer, (void*)&task);
        }
        
        // Clean up the threads
        BOOST_FOREACH(shared_ptr<pthread_t> & t, threads)
            pthread_join(*t, NULL);

        StopServer(*thread, port);
        return 1;
    }

    int TestRetrieveExample() {
        int port = 9596;
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        config.SetInt("feature_n", 1);
        config.SetString("learner", "perceptron");
        shared_ptr<pthread_t> thread = StartServer(config);
        ostringstream url; url << "http://localhost:" << port << "/RPC2";
        string server_url = url.str();
        
        // Get a task
        int task = GetTask(server_url);
        
        // Add unlabeled values
        {
            params_t params;
            params["id"] = value_int(0);
            params["text"] = value_string("テ ル ト 2");
            params["task_id"] = value_int(task);
            params["lab"] = value_int(1);
            CallServer(server_url, "add_labeled", params);
        }
        
        // Add unlabeled values
        {
            params_t params;
            params["id"] = value_int(1);
            params["task_id"] = value_int(task);
            params["text"] = value_string("テ ス ト 1");
            CallServer(server_url, "add_unlabeled", params);
        }
        {
            params_t params;
            params["id"] = value_int(2);
            params["task_id"] = value_int(task);
            params["text"] = value_string("テ ス ト 2");
            CallServer(server_url, "add_unlabeled", params);
        }

        // Check to make sure that we get the best scored one with the proper
        // values
        params_t params_ret;
        {
            params_t params;
            params["task_id"] = value_int(task);
            params_ret = value_struct(CallServer(server_url,"pop_best",params));
        }
        StopServer(*thread, port);
        // Check that the return matches our expected value
        params_t params;
        params["id"] = value_int(2);
        params["text"] = value_string("テ ス ト 2");
        params["lab"] = value_int(-1);
        params["score"] = value_double(4);
        return CheckParamsMap(params, params_ret);
    }

    int TestAddKeyword() {
        int port = 9595;
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        config.SetInt("feature_n", 1);
        shared_ptr<pthread_t> thread = StartServer(config);
        ostringstream url; url << "http://localhost:" << port << "/RPC2";
        string server_url = url.str();
        clientSimple my_client;
        
        int task = GetTask(server_url);

        // Add keywords twice, this should give us a 10 for each feature
        for(int i = 0; i < 2; i++) {
            params_t params;
            params["id"] = value_int(0);
            params["task_id"] = value_int(task);
            params["text"] = value_string("this was a pen");
            params["lab"] = value_int(1);
            CallServer(server_url, "add_keyword", params);
        }
        // Add an unlabeled example, this should not add additional weights to 
        // the model
        {
            params_t params;
            params["id"] = value_int(0);
            params["task_id"] = value_int(task);
            params["text"] = value_string("this is a girl");
            CallServer(server_url, "add_unlabeled", params);
        }
        // Get the weights
        params_t weights_act, weights_exp;
        {
            params_t params;
            params["id"] = value_int(1);
            params["task_id"] = value_int(task);
            weights_act = value_struct(CallServer(server_url,"get_weights",params));
            weights_exp["this"] = value_int(10);
            weights_exp["was"] = value_int(10);
            weights_exp["a"] = value_int(10);
            weights_exp["pen"] = value_int(10);
        }
        // Get the keywords
        array_t keywords_act, keywords_exp;
        {
            params_t params;
            params["task_id"] = value_int(task);
            keywords_act = array_t(value_array(CallServer(server_url,"get_keywords",params)).vectorValueValue());
            keywords_exp.push_back(value_string("this was a pen"));
        }

        StopServer(*thread, port);
        
        return CheckParamsMap(weights_exp, weights_act) &&
                CheckValueArray(keywords_exp, keywords_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestStartStop()" << endl; if(TestStartStop()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRetrieveExample()" << endl; if(TestRetrieveExample()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestAddKeyword()" << endl; if(TestAddKeyword()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestThrashServer()" << endl; if(TestThrashServer()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestServer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
