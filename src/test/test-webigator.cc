#include <iostream>
#include <vector>
#include <webigator/util.h>
#include "test-data-store.h"
#include "test-text-classifier.h"
#include "test-server.h"
#include "test-base.h"

using namespace std;
using namespace webigator;

int main() {
    // Initialize all the tests
    vector<TestBase*> tests;
    tests.push_back(new TestDataStore());
    tests.push_back(new TestTextClassifier());
    tests.push_back(new TestServer());
    // Run all the tests
    GlobalVars::debug = 2;
    int number_passed = 0;
    for(int i = 0; i < (int)tests.size(); i++)
        if(tests[i]->RunTest())
            number_passed++;
    // Check whether all were passed or not
    if(number_passed == (int)tests.size()) {
        cout << "**** passed ****" << endl;
    } else {
        cout << "**** FAILED!!! ****" << endl;
    }
    // Deallocate
    for(int i = 0; i < (int)tests.size(); i++)
        delete tests[i]; 
}
