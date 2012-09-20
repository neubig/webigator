#include <webigator/text-example.h>
#include <cfloat>

using namespace webigator;
using namespace std;

// Output for a hyperedge in JSON format
void TextExample::Print(std::ostream & out) const {
    out << "{\"id\": "<<id_;
    if(str_.length()) {
        out << ", \"str\": [";
        for(int i = 0; i < (int)str_.length(); i++)
            out << '"' << Dict::WSym(str_[i]) << '"' << ((i == (int)str_.length()-1) ? "]" : ", ");
    }
    if(label_ != -1)
        out << ", \"label\": " << label_;
    if(score_ != -DBL_MAX)
        out << ", \"score\": " << score_;
    out << "}";
}
