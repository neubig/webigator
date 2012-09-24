#ifndef TEST_UTIL_H__
#define TEST_UTIL_H__

#include "test-base.h"
#include <boost/shared_ptr.hpp>
#include <webigator/text-classifier.h>
#include <webigator/text-example.h>

using namespace boost;

namespace webigator {

class TestUtil : public TestBase {

public:

    TestUtil() { }

    ~TestUtil() { }

    int TestNormalizeLogProbs() {
        vector<double> vals(2);
        vals[0] = 0;
        vals[1] = -100;
        NormalizeLogProbs(vals);
        cerr << vals[0] << ", " << vals[1] << endl;
        return vals[0] != 0;
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestNormalizeLogProbs()" << endl; if(TestNormalizeLogProbs()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestUtil Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
