#include <iostream>
#include "test-file-loader.h"
#include "test-model-one-probs.h"
#include "test-nafil-runner.h"
#include "test-base.h"

using namespace std;
using namespace nafil;

int main() {
    // Initialize all the tests
    vector<TestBase*> tests;
    tests.push_back(new TestFileLoader());
    tests.push_back(new TestModelOneProbs());
    tests.push_back(new TestNafilRunner());
    // Run all the tests
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
