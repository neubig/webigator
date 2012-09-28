#ifndef TEST_BASE__
#define TEST_BASE__

#include <vector>
#include <set>
#include <iostream>
#include <cmath>
#include <tr1/unordered_map>
#include <boost/foreach.hpp>

using namespace std;

namespace webigator {

class TestBase {

public:

    TestBase() : passed_(false) { }
    virtual ~TestBase() { }

    // RunTest must be implemented by any test, and returns true if all
    // tests were passed
    virtual bool RunTest() = 0;

protected:

    bool passed_;

    template<class T>
    int CheckVector(const std::vector<T> & exp, const std::vector<T> & act) {
        int ok = 1;
        for(int i = 0; i < (int)max(exp.size(), act.size()); i++) {
            if(i >= (int)exp.size() || 
               i >= (int)act.size() || 
               exp[i] != act[i]) {
               
                ok = 0;
                std::cout << "exp["<<i<<"] != act["<<i<<"] (";
                if(i >= (int)exp.size()) std::cout << "NULL";
                else std::cout << exp[i];
                std::cout <<" != ";
                if(i >= (int)act.size()) std::cout << "NULL"; 
                else std::cout << act[i];
                std::cout << ")" << std::endl;
            }
        }
        return ok;
    }

    template<class K, class V>
    int CheckMap(const std::tr1::unordered_map<K,V> & exp, const std::tr1::unordered_map<K,V> & act) {
        typedef std::tr1::unordered_map<K,V> MapType;
        typedef std::pair<K,V> MapPair;
        int ok = 1;
        BOOST_FOREACH(MapPair kv, exp) {
            typename MapType::const_iterator it = act.find(kv.first);
            if(it == act.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << endl;
                ok = 0;
            } else if(it->second != kv.second) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != "<<it->second<<")" << endl;
                ok = 0;
            }
        }
        BOOST_FOREACH(MapPair kv, act) {
            typename MapType::const_iterator it = exp.find(kv.first);
            if(it == act.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << endl;
                ok = 0;
            }
        }
        return ok;
    }

    template<class K>
    int CheckAlmostMap(const std::tr1::unordered_map<K,double> & exp, const std::tr1::unordered_map<K,double> & act) {
        typedef std::tr1::unordered_map<K,double> MapType;
        typedef std::pair<K,double> MapPair;
        int ok = 1;
        BOOST_FOREACH(MapPair kv, exp) {
            typename MapType::const_iterator it = act.find(kv.first);
            if(it == act.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << endl;
                ok = 0;
            } else if(abs(it->second - kv.second) > 0.01) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != "<<it->second<<")" << endl;
                ok = 0;
            }
        }
        BOOST_FOREACH(MapPair kv, act) {
            typename MapType::const_iterator it = exp.find(kv.first);
            if(it == act.end()) {
                std::cout << "exp["<<kv.first<<"] != act["<<kv.first<<"] ("<<kv.second<<" != NULL)" << endl;
                ok = 0;
            }
        }
        return ok;
    }

    template<class V>
    int CheckSet(const std::set<V> & exp, const std::set<V> & act) {
        int ok = 1;
        BOOST_FOREACH(const V & v, exp) {
            typename std::set<V>::const_iterator it = act.find(v);
            if(it == act.end()) {
                std::cout << "exp != act ("<<v<<" != NULL)" << endl;
                ok = 0;
            }
        }
        BOOST_FOREACH(const V & v, act) {
            typename std::set<V>::const_iterator it = exp.find(v);
            if(it == act.end()) {
                std::cout << "exp != act ("<<v<<" != NULL)" << endl;
                ok = 0;
            }
        }
        return ok;
    }

    template<class T>
    int CheckAlmostVector(const std::vector<T> & exp,
                          const std::vector<T> & act) {
        int ok = 1;
        for(int i = 0; i < (int)max(exp.size(), act.size()); i++) {
            if(i >= (int)exp.size() || 
               i >= (int)act.size() || 
               abs(exp[i] - act[i]) > 0.01) {
               
                ok = 0;
                std::cout << "exp["<<i<<"] != act["<<i<<"] (";
                if(i >= (int)exp.size()) std::cout << "NULL";
                else std::cout << exp[i];
                std::cout <<" != ";
                if(i >= (int)act.size()) std::cout << "NULL"; 
                else std::cout << act[i];
                std::cout << ")" << std::endl;
            }
        }
        return ok;
    }

    int CheckEqual(double exp, double act) {
        if(exp != act) {
            std::cout << "CheckEqual: " << exp << " != " << act << endl;
            return 0;
        }
        return 1;
    }

    int CheckAlmost(double exp, double act) {
        if(abs(exp - act) > 0.01) {
            std::cout << "CheckAlmost: " << exp << " != " << act << endl;
            return 0;
        }
        return 1;
    }

    int CheckString(const std::string & exp, const std::string & act) {
        if(exp != act) {
            cerr << "CheckString failed" << endl << "exp: '"<<exp<<"'"
                 <<endl<<"act: '"<<act<<"'" <<endl;
            for(int i = 0; i < (int)min(exp.length(), act.length()); i++)
                if(exp[i] != act[i])
                    cerr << "exp[" << i << "] '" << exp[i] << "' != act["<<i<<"] '"<<act[i]<<"'" <<endl;
            return 0;
        }
        return 1;
    }

};

}

#endif
