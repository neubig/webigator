#ifndef TEST_DATA_STORE_H__
#define TEST_DATA_STORE_H__

#include "test-base.h"
#include <boost/shared_ptr.hpp>
#include <webigator/data-store.h>
#include <webigator/text-example.h>

using namespace boost;

namespace webigator {

class TestDataStore : public TestBase {

public:

    TestDataStore() {
    }

    ~TestDataStore() { }

    // Check to make sure that adding new examples results in a proper
    // number of examples in the cache and proper score order
    int TestAddNewExample() {
        DataStore store;
        store.SetMaxCacheSize(2);
        for(int i = 0; i < 4; i++) {
            ostringstream oss; oss << "test " << i;
            TextExample exp(i, oss.str(), -1, i);
            store.AddNewExample(exp);
            if(!CheckEqual(std::min(i+1, 2), store.GetCacheSize()))
                return 0;
        }
        return 
            CheckEqual(2, store.GetCacheSize()) && 
            CheckEqual(3, store.PeekNextExample().GetScore());
    }
    
    // Test to make sure that even if the same example is added twice,
    // that it should only be included in the cache once, but the score
    // should be updated
    int TestAddExampleTwice() {
        DataStore store;
        // Add the first one
        TextExample exp0(0, "test1", -1, 0);
        store.AddNewExample(exp0); 
        if(!CheckEqual(1, store.GetCacheSize()) ||
           !CheckEqual(0, store.PeekNextExample().GetScore()))
            return 0;
        // Add the second one
        TextExample exp1(1, "test2", -1, 1);
        store.AddNewExample(exp1);
        if(!CheckEqual(2, store.GetCacheSize()) ||
           !CheckEqual(1, store.PeekNextExample().GetScore()))
            return 0;
        // Add the first one again with a higher score
        exp0.SetScore(2);
        store.AddNewExample(exp0); 
        if(!CheckEqual(2, store.GetCacheSize()) ||
           !CheckEqual(2, store.PeekNextExample().GetScore()))
            return 0;
        return 1;
    }
    
    // Test whether PopNextExample gets the example with the best score
    int TestPopNextExample() {
        DataStore store;
        store.SetMaxCacheSize(2);
        TextExample exp0(0, "test", -1, 0); store.AddNewExample(exp0); 
        TextExample exp1(1, "test", -1, 1); store.AddNewExample(exp1);
        TextExample exp2(2, "test", -1, 2); store.AddNewExample(exp2);
        return CheckEqual(2, store.PopNextExample().GetScore()) &&
               CheckEqual(1, store.PeekNextExample().GetScore());
    }

    // Test whether we can rescore the hash appropriately
    int TestRescoreCache() {
        DataStore store;
        store.SetMaxCacheSize(2);
        for(int i = 0; i < 4; i++) {
            ostringstream oss; oss << "test " << i;
            TextExample exp(i, oss.str(), -1, i);
            store.AddNewExample(exp);
        }
        TextClassifier classifier(2, 1, Classifier::PERCEPTRON);
        classifier.GetBinaryWeights().insert(MakePair(Dict::FeatID("2"), 5));
        store.RescoreCache(classifier);
        return 
            CheckEqual(2, store.GetCacheSize()) && 
            CheckEqual(5, store.PeekNextExample().GetScore());
    }

    // int TestTimeDecay() {
    //     // TODO: This should test that scores decay properly over time
    //     return 0;
    // }
    // 
    // int TestResetTimeDecay() {
    //     // TODO: This should test that we can make the times decay properly
    //     return 0;
    // }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestAddNewExample()" << endl; if(TestAddNewExample()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestAddExampleTwice()" << endl; if(TestAddExampleTwice()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestPopNextExample()" << endl; if(TestPopNextExample()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestRescoreCache()" << endl; if(TestRescoreCache()) succeeded++; else cout << "FAILED!!!" << endl;
        // done++; cout << "TestTimeDecay()" << endl; if(TestTimeDecay()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestDataStore Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
