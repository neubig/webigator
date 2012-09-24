#include <webigator/text-classifier.h>
#include <webigator/sparse-map.h>
#include <boost/foreach.hpp>
#include <numeric>

using namespace webigator;
using namespace std;

SparseMap TextClassifier::CalculateFeatures(const TextExample & exp, bool add) {
    SparseMap ret;
    const GenericString<int> & str = exp.GetString();
    for(int i = 0; i < (int)str.length(); i++) {
        for(int j = i+1; j <= (int)str.length(); j++) {
            if(j-i > feature_n_) break;
            ret[Dict::FeatID(str.substr(i, j-i), add)]++;
        }
    }
    return ret;
}

void TextClassifier::UpdateWithLabeledExample(const TextExample & exp,
                                              Classifier::Learner update) {
    if(update == Classifier::UNSPECIFIED)
        update = learner_;
    // Calculate the features
    SparseMap features = CalculateFeatures(exp);
    // Currently only support the perceptron update
    if(update == Classifier::PERCEPTRON) {
        // Calculate the score for all of our values
        vector<double> scores = GetScores(features);
        // If we are doing binary or unary classification
        if(weights_.size() == 1) {
            int y = (exp.GetLabel() * 2 - 1); // Convert the label to 1, -1
            if(y*scores[1] <= 0)              // If we are wrong
                weights_[0] += features * y;  // Do the perceptron update
        } else {
            THROW_ERROR("Not implemeted yet");
        }
    } else if(update == Classifier::KEYWORD) {
        if(weights_.size() == 1) {
            int y = (exp.GetLabel() * 2 - 1); // Convert the label to 1, -1
            weights_[0] += features * y * keyword_weight_;  // Do the keyword update
        } else {
            weights_[exp.GetLabel()] += features * keyword_weight_;
        }
    } else if(update == Classifier::NAIVE_BAYES) {
        weights_[exp.GetLabel()] += features;
    } else {
        THROW_ERROR("Illegal update type");
    }
}


std::vector<double> TextClassifier::GetScores(const SparseMap & features) const {
    std::vector<double> ret;
    // Currently only support the perceptron update
    if(learner_ == Classifier::PERCEPTRON) {
        BOOST_FOREACH(const SparseMap & weight, weights_)
            ret.push_back(features * weight);
        // For binary classification, recover
        if(ret.size() == 1) {
            ret.push_back(ret[0]);
            ret[0] *= -1;
        }
    } else if (learner_ == Classifier::NAIVE_BAYES) {
        ret.resize(weights_.size());
        BOOST_FOREACH(const SparseMap::value_type val, features) {
            std::vector<double> counts(weights_.size(), dirichlet_alpha_);
            for(int i = 0; i < (int)weights_.size(); i++) {
                SparseMap::const_iterator it = weights_[i].find(val.first);
                if(it != weights_[i].end())
                    counts[i] += it->second;
            }
            double log_count = log(std::accumulate(counts.begin(), counts.end(), 0.0));
            for(int i = 0; i < (int)weights_.size(); i++)
                ret[i] += log(counts[i]) - log_count;
        }
        NormalizeLogProbs(ret);
    } else {
        THROW_ERROR("Unsupported classifier type in GetScores");
    }
    return ret;
}
