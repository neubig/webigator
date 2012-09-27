#ifndef WEBIGATOR_DICT_H__
#define WEBIGATOR_DICT_H__

#include <string>
#include <vector>
#include <climits>
#include <cstdlib>
#include <boost/algorithm/string.hpp>
#include <webigator/symbol-set.h>
#include <webigator/util.h>
#include <webigator/sparse-map.h>
#include <webigator/generic-string.h>

namespace webigator {

typedef int WordId;
typedef std::vector<WordId> Sentence;

struct Dict {

    // Get the word ID
    static WordId WID(const std::string & str, bool add = true) {
        return wids_.GetId(str, add);
    }
    // Get the feature ID
    static WordId FeatID(const GenericString<WordId> & str, bool add = true) {
        return fids_.GetId(str.str(), add);
    }
    static WordId FeatID(const std::string & str, bool add = true) {
        return FeatID(GenericString<WordId>(ParseWords(str, add)), add);
    }

    // Get the quoted word ID
    static WordId QuotedWID(const std::string & str, bool add = true) {
        // For x0 -> -1, x1 -> -2, etc.
        if(str[0] == 'x') {
            return -1-atoi(str.substr(1).c_str());
        // Otherwise, string must be quoted
        } else if (str[0] == '"' && str.length() > 2 && str[str.length()-1] == '"') {
            return wids_.GetId(str.substr(1,str.length()-2), add);
        } else {
            THROW_ERROR("Bad quoted string at " << str);
            return INT_MIN;
        }
    }

    // Get the word symbol
    static const std::string & WSym(WordId id) {
        return wids_.GetSymbol(id);
    }
    static GenericString<WordId> FeatSym(WordId id) {
        return GenericString<WordId>(fids_.GetSymbol(id));
    }
    
    // Get the word symbol
    static std::string WSymEscaped(WordId id) {
        std::string ret = wids_.GetSymbol(id);
        boost::replace_all(ret, "\"", "\\\"");
        return ret;
    }

    // Get the
    static std::string WAnnotatedSym(WordId id) {
        std::ostringstream oss;
        if(id < 0)
            oss << "x" << -1+id*-1;
        else
            oss << '"' << wids_.GetSymbol(id) << '"';
        return oss.str();
    }

    static std::string PrintWords(const GenericString<WordId> & ids) {
        std::ostringstream oss;
        for(int i = 0; i < (int)ids.length(); i++) {
            if(i != 0) oss << ' ';
            oss << WSym(ids[i]);
        }
        return oss.str();
    }
    static std::string PrintWords(const std::vector<WordId> & ids) {
        std::ostringstream oss;
        for(int i = 0; i < (int)ids.size(); i++) {
            if(i != 0) oss << ' ';
            oss << WSym(ids[i]);
        }
        return oss.str();
    }

    static std::string PrintAnnotatedWords(const std::vector<WordId> & ids) {
        std::ostringstream oss;
        for(int i = 0; i < (int)ids.size(); i++) {
            if(i != 0) oss << ' ';
            oss << WAnnotatedSym(ids[i]);
        }
        return oss.str();
    }


    // Feature functions
    static std::string PrintFeatures(const SparseMap & feats);
    static SparseMap ParseFeatures(std::istream & iss);
    static SparseMap ParseFeatures(const std::string & str);

    // Get the word ID
    static std::vector<WordId> ParseWords(const std::string & str, bool add = true) {
        std::istringstream iss(str);
        std::string buff;
        std::vector<WordId> ret;
        while(iss >> buff)
            ret.push_back(WID(buff, add));
        return ret;
    }
    
    // Get the word ID
    static std::vector<WordId> ParseQuotedWords(const std::string & str, bool add = true) {
        std::istringstream iss(str);
        std::string buff;
        std::vector<WordId> ret;
        while(iss >> buff)
            ret.push_back(QuotedWID(buff, add));
        return ret;
    }

private:
    static SymbolSet<WordId> wids_;
    static SymbolSet<WordId> fids_;

};

}

#endif
