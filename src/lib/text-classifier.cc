#include <webigator/text-classifier.h>
#include <webigator/sparse-map.h>
#include <boost/foreach.hpp>

using namespace webigator;
using namespace std;

SparseMap TextClassifier::CalculateFeatures(const TextExample & exp) {
    SparseMap ret;
    const GenericString<int> & str = exp.GetString();
    for(int i = 0; i < (int)str.length(); i++) {
        for(int j = i+1; j <= (int)str.length(); j++) {
            if(j-i > feature_n_) break;
            ret[Dict::FeatID(str.substr(i, j-i))]++;
        }
    }
    return ret;
}

void TextClassifier::UpdateWithLabeledExample(const TextExample & exp) {
    // Currently only support the perceptron update
    if(update_type_ != Classifier::PERCEPTRON)
        THROW_ERROR("Illegal update type in UpdateWithLabeledExample");
    // Calculate the features
    SparseMap features = CalculateFeatures(exp);
    // Calculate the score for all of our values
    vector<double> scores = GetScores(features);
    // If we are doing binary or unary classification
    if(scores.size() == 1) {
        int y = (exp.GetLabel() * 2 - 1); // Convert the label to 1, -1
        if(y*scores[0] <= 0)              // If we are wrong
            weights_[0] += features * y;  // Do the perceptron update
    } else {
        THROW_ERROR("Not implemeted yet");
    }
}


std::vector<double> TextClassifier::GetScores(const SparseMap & features) {
    std::vector<double> ret;
    BOOST_FOREACH(const SparseMap & weight, weights_)
        ret.push_back(features * weight);
    return ret;
}
