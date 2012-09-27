#ifndef TEXT_CLASSIFIER_H__
#define TEXT_CLASSIFIER_H__

#include <webigator/classifier.h>
#include <webigator/sparse-map.h>
#include <webigator/text-example.h>
#include <vector>

namespace webigator {

class TextClassifier : public Classifier {

public:
    TextClassifier(int num_classes = 2,
                   int feature_n = 1, 
                   Classifier::Learner learner = Classifier::NAIVE_BAYES) :
                        num_classes_(num_classes), feature_n_(feature_n),
                        keyword_weight_(5), dirichlet_alpha_(0.5),
                        learner_(learner) , weights_( (num_classes <= 2 && learner != Classifier::NAIVE_BAYES) ? 1 : num_classes) { }

    // Add a labeled example exp using the update rule curr_update
    // If the update rule is unspecified, use the default rule. The
    // specification of the update rule is basically to allow for specification of keyword
    void UpdateWithLabeledExample(const TextExample & exp,
                                  Classifier::Learner curr_update = Classifier::UNSPECIFIED);

    const SparseMap & GetWeights(int id) const { return weights_[id]; }
    SparseMap & GetWeights(int id) { return weights_[id]; }
    const SparseMap & GetBinaryWeights() const { return weights_[0]; }
    SparseMap & GetBinaryWeights() { return weights_[0]; }
    double GetKeywordWeight() const { return keyword_weight_; }
    void SetKeywordWeight(double keyword_weight) { keyword_weight_ = keyword_weight; }
    double GetDirichletAlpha() const { return dirichlet_alpha_; }
    void SetDirichletAlpha(double dirichlet_alpha) { dirichlet_alpha_ = dirichlet_alpha; }

    SparseMap CalculateFeatures(const TextExample & exp, bool add = true);
    const SparseMap CalculateFeatures(const TextExample & exp) const {
        return ((TextClassifier*)this)->CalculateFeatures(exp, false);
    }

    std::vector<double> GetScores(const SparseMap & features) const;
    std::vector<double> GetScores(const TextExample & exp) const {
        return GetScores(CalculateFeatures(exp));
    }
    double GetBinaryScore(const TextExample & exp) const {
        std::vector<double> ret = GetScores(exp);
        if(ret.size() != 2) THROW_ERROR("Attempting to get binary score for non-binary classifier");
        return ret[1];
    }
    double GetBinaryMargin(const TextExample & exp) const {
        std::vector<double> ret = GetScores(exp);
        if(ret.size() != 2) THROW_ERROR("Attempting to get binary score for non-binary classifier");
        return ret[1]-ret[0];
    }

    const Classifier::Learner GetLearner() const { return learner_; }
    void SetLearner(Classifier::Learner learner) { learner_ = learner; }

private:
    // The number of classes to divide things into
    int num_classes_;
    // The n-gram length of features
    int feature_n_;
    // The weight to assign to keywords
    double keyword_weight_;
    // The alpha for a dirichlet prior
    double dirichlet_alpha_;
    // The type of update rule to use for training
    Classifier::Learner learner_;
    // Weights to use
    std::vector<SparseMap> weights_;


};

}

#endif
