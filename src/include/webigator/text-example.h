#ifndef TEXT_EXAMPLE_H__
#define TEXT_EXAMPLE_H__

#include <webigator/dict.h>
#include <webigator/generic-string.h>
#include <boost/shared_ptr.hpp>
#include <cfloat>

namespace webigator {

typedef long long ExampleId;

class TextExample {

public:
    TextExample(ExampleId id = -1, const std::string & str = "", int label = -1, double score = -DBL_MAX) :
        id_(id),
        str_(Dict::ParseWords(str)),
        label_(label),
        score_(score) { }

    // Input/Output
    void Print(std::ostream & out) const;

    void SetLabel(int label) { label_ = label; }
    int GetLabel() const { return label_; }
    const GenericString<int> & GetString() const { return str_; }
    GenericString<int> & GetString() { return str_; }
    double GetScore() const { return score_; }
    void SetScore(double score) { score_ = score; }
    ExampleId GetId() const { return id_; }
    void SetId(ExampleId id) { id_ = id; }

private:
    ExampleId id_;
    GenericString<int> str_;
    int label_;
    double score_;

};

inline std::ostream &operator<<( std::ostream &out, const TextExample &L ) {
    L.Print(out);
    return out;
}

class TextExampleScoreMore {
public:
    bool operator()(const TextExample & x, const TextExample & y) {
        if(x.GetScore() != y.GetScore()) return x.GetScore() > y.GetScore();
        return x.GetId() > y.GetId();
    }
    bool operator()(const boost::shared_ptr<TextExample> & x, const boost::shared_ptr<TextExample> & y) {
        if(x->GetScore() != y->GetScore()) return x->GetScore() > y->GetScore();
        return x->GetId() > y->GetId();
    }
};



}

#endif
