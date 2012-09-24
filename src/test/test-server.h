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

    pthread_t StartServer(int port) {
        // Start the server running
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        return StartServer(config);
    }
    pthread_t StartServer(const ConfigWebigatorServer & config) {
        pthread_t thread;
        pthread_create( &thread, NULL, run_server_function, (void*)&config);
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
        pthread_t thread = StartServer(port);
        StopServer(thread, port);
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
            typename params_t::const_iterator it = act.find(kv.first);
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
            typename params_t::const_iterator it = exp.find(kv.first);
            if(it == exp.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<
                            "] (NULL != "<<ValToString(kv.second)<<")" << endl;
                ok = 0;
            }
        }
        return ok;
    }

    int TestRetrieveExample() {
        int port = 9596;
        ConfigWebigatorServer config;
        config.SetInt("port", port);
        config.SetString("learner", "perceptron");
        pthread_t thread = StartServer(config);
        ostringstream url; url << "http://localhost:" << port << "/RPC2";
        string server_url = url.str();
        
        // Add unlabeled values
        {
            clientSimple my_client;
            value result;
            params_t params;
            params["id"] = value_int(0);
            params["text"] = value_string("テ ル ト 2");
            params["lab"] = value_int(1);
            paramList myParamList;
            myParamList.add(value_struct(params));
            my_client.call(server_url, "add_labeled", myParamList, &result);
        }
        
        // Add unlabeled values
        params_t params1, params2, params_ret;
        {
            clientSimple my_client;
            value result;
            params1["id"] = value_int(1);
            params1["text"] = value_string("テ ス ト 1");
            paramList myParamList;
            myParamList.add(value_struct(params1));
            my_client.call(server_url, "add_unlabeled", myParamList, &result);
        }
        {
            clientSimple my_client;
            value result;
            params2["id"] = value_int(2);
            params2["text"] = value_string("テ ス ト 2");
            paramList myParamList;
            myParamList.add(value_struct(params2));
            my_client.call(server_url, "add_unlabeled", myParamList, &result);
        }

        // Check to make sure that we get the best scored one with the proper values
        clientSimple my_client;
        value result;
        paramList myParamList;
        my_client.call(server_url, "pop_best", myParamList, &result);
        params_ret = value_struct(result);
        StopServer(thread, port);
        params2["lab"] = value_int(-1);
        params2["score"] = value_double(3);
        return CheckParamsMap(params2, params_ret);
    }

    int TestAddKeyword() {
        int port = 9595;
        pthread_t thread = StartServer(port);
        ostringstream url; url << "http://localhost:" << port << "/RPC2";
        string server_url = url.str();
        
        // Add keywords twice, this should give us a 10 for each feature
        for(int i = 0; i < 2; i++) {
            clientSimple my_client;
            value result;
            params_t params;
            params["id"] = value_int(0);
            params["text"] = value_string("this was a pen");
            params["lab"] = value_int(1);
            paramList myParamList;
            myParamList.add(value_struct(params));
            my_client.call(server_url, "add_keyword", myParamList, &result);
        }
        // Add an unlabeled example, this should not add additional weights to the model
        {
            clientSimple my_client;
            value result;
            params_t params2;
            params2["id"] = value_int(2);
            params2["text"] = value_string("this is a girl");
            paramList myParamList;
            myParamList.add(value_struct(params2));
            my_client.call(server_url, "add_unlabeled", myParamList, &result);
        }
        // Get the weights
        clientSimple my_client;
        value result;
        paramList myParamList;
        myParamList.add(value_int(1));
        my_client.call(server_url, "get_weights", myParamList, &result);
        params_t weights_act = value_struct(result), weights_exp;
        weights_exp["this"] = value_int(10);
        weights_exp["was"] = value_int(10);
        weights_exp["a"] = value_int(10);
        weights_exp["pen"] = value_int(10);
        StopServer(thread, port);
        
        return CheckParamsMap(weights_exp, weights_act);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestStartStop()" << endl; if(TestStartStop()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRetrieveExample()" << endl; if(TestRetrieveExample()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestAddKeyword()" << endl; if(TestAddKeyword()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestServer Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
