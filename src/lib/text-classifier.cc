#include <webigator/text-classifier.h>
#include <webigator/sparse-map.h>
#include <boost/foreach.hpp>
#include <numeric>

using namespace webigator;
using namespace std;

SparseMap TextClassifier::CalculateFeatures(const TextExample & exp, bool use_len, bool add) {
    SparseMap ret;
    const GenericString<int> & str = exp.GetString();
    // Add n-gram features
    for(int i = 0; i < (int)str.length(); i++) {
        for(int j = i+1; j <= (int)str.length(); j++) {
            if(j-i > feature_n_) break;
            ret[Dict::FeatID(str.substr(i, j-i), add)]++;
        }
    }
    // Add length features
    if(use_len) {
        ostringstream oss; oss << "<LENGTH" << exp.GetLength() / 10 * 10 << ">";
        ret[Dict::FeatID(GenericString<int>(1, Dict::WID(oss.str())))]++;
    }
    return ret;
}

void TextClassifier::UpdateWithLabeledExample(const TextExample & exp,
                                              Classifier::Learner update) {
    if(update == Classifier::UNSPECIFIED)
        update = learner_;
    // Calculate the features
    SparseMap features = CalculateFeatures(exp, use_length_ && (update != Classifier::KEYWORD));
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
            // BOOST_FOREACH(SparseMap::value_type & val, features) {
            //     cerr << "DEBUG key: weights_[" << exp.GetLabel() << ", " << Dict::PrintWords(Dict::FeatSym(val.first)) << "] += " << val.second << " * " << keyword_weight_ << endl;
            // }
            weights_[exp.GetLabel()] += features * keyword_weight_;
        }
    } else if(update == Classifier::NAIVE_BAYES) {
        // BOOST_FOREACH(SparseMap::value_type & val, features) {
        //     cerr << "DEBUG val: weights_[" << exp.GetLabel() << ", " << Dict::PrintWords(Dict::FeatSym(val.first)) << "] += " << val.second << endl;
        // }
        weights_[exp.GetLabel()] += features;
    } else {
        THROW_ERROR("Illegal update type");
    }
}


std::vector<double> TextClassifier::GetScores(const SparseMap & features) const {
    std::vector<double> ret;
    if(learner_ == Classifier::PERCEPTRON) {
        BOOST_FOREACH(const SparseMap & weight, weights_)
            ret.push_back(features * weight);
        // For binary classification, recover
        if(ret.size() == 1) {
            ret.push_back(ret[0]);
            ret[0] = 0;
        }
    } else if (learner_ == Classifier::NAIVE_BAYES) {
        ret.resize(weights_.size());
        // Get the label alpha
        vector<double> label_prior(label_counts_);
        BOOST_FOREACH(double & count, label_prior) {
            count = (count + label_alpha_ / label_counts_.size()) / (label_total_+label_alpha_);
        }
        BOOST_FOREACH(const SparseMap::value_type val, features) {
            // Get the feature alpha using this 
            std::vector<double> counts(label_prior);
            for(int i = 0; i < (int)weights_.size(); i++) {
                SparseMap::const_iterator it = weights_[i].find(val.first);
                if(it != weights_[i].end())
                    counts[i] += it->second;
            }
            double log_count = log(std::accumulate(counts.begin(), counts.end(), 0.0));
            for(int i = 0; i < (int)weights_.size(); i++)
                ret[i] += (log(counts[i]) - log_count) * val.second;
        }
        NormalizeLogProbs(ret);
    } else {
        THROW_ERROR("Unsupported classifier type in GetScores");
    }
    return ret;
}
