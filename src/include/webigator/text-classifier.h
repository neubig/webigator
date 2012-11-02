#ifndef TEXT_CLASSIFIER_H__
#define TEXT_CLASSIFIER_H__

#include <webigator/classifier.h>
#include <webigator/sparse-map.h>
#include <webigator/text-example.h>
#include <vector>

namespace webigator {

class TextClassifier : public Classifier {

public:

    typedef enum {
        UNNORMALIZED,
        GEOM_MEAN,
        SQRT_GEOM_MEAN
    } NormalizeType;

    TextClassifier(int num_classes = 2,
                   int feature_n = 1, 
                   Classifier::Learner learner = Classifier::NAIVE_BAYES) :
                        num_classes_(num_classes), feature_n_(feature_n),
                        use_length_(true),
                        keyword_weight_(5),
                        feature_alpha_(1), label_alpha_(1),
                        label_total_(0),
                        label_counts_(num_classes),
                        learner_(learner), 
                        weights_( (num_classes <= 2 && learner != Classifier::NAIVE_BAYES) ? 1 : num_classes)
            { }

    // Add a labeled example exp using the update rule curr_update
    // If the update rule is unspecified, use the default rule. The
    // specification of the update rule is basically to allow for specification of keyword
    void UpdateWithLabeledExample(const TextExample & exp,
                                  Classifier::Learner curr_update = Classifier::UNSPECIFIED);

    int GetFeatureN() const { return feature_n_; }
    void SetFeatureN(int feature_n) { feature_n_ = feature_n; }
    bool GetUseLength() const { return use_length_; }
    void SetUseLength(bool use_length) { use_length_ = use_length; }

    const SparseMap & GetWeights(int id) const { return SafeAccess(weights_, id); }
    SparseMap & GetWeights(int id) { return SafeAccess(weights_, id); }
    const SparseMap & GetBinaryWeights() const { return weights_[0]; }
    SparseMap & GetBinaryWeights() { return weights_[0]; }
    double GetKeywordWeight() const { return keyword_weight_; }
    void SetKeywordWeight(double keyword_weight) { keyword_weight_ = keyword_weight; }
    double GetFeatureAlpha() const { return feature_alpha_; }
    void SetFeatureAlpha(double feature_alpha) { feature_alpha_ = feature_alpha; }
    double GetLabelAlpha() const { return label_alpha_; }
    void SetLabelAlpha(double label_alpha) { label_alpha_ = label_alpha; }
    double GetLabelCount(int id) const { return label_counts_[id]; }
    void SetLabelCount(int id, double label_count) { 
        label_total_ += label_count-label_counts_[id];
        label_counts_[id] = label_count;
    }
    void AddLabelCount(int id, double label_count) { 
        label_total_ += label_count;
        label_counts_[id] += label_count;
    }

    SparseMap CalculateFeatures(const TextExample & exp, bool use_len, bool add = true);
    const SparseMap CalculateFeatures(const TextExample & exp, bool use_len) const {
        return ((TextClassifier*)this)->CalculateFeatures(exp, use_len, false);
    }

    double Normalize(double score, NormalizeType norm, int len) const {
        if(norm == GEOM_MEAN)
            score /= len;
        else if(norm == SQRT_GEOM_MEAN)
            score /= sqrt(len);
        return score;
    }

    std::vector<double> GetScores(const SparseMap & features) const;
    std::vector<double> GetScores(const TextExample & exp) const {
        return GetScores(CalculateFeatures(exp, use_length_));
    }
    double GetBinaryScore(const TextExample & exp, NormalizeType norm = UNNORMALIZED) const {
        std::vector<double> ret = GetScores(exp);
        if(ret.size() != 2) THROW_ERROR("Attempting to get binary score for non-binary classifier");
        return Normalize(ret[1], norm, exp.GetLength());
    }
    double GetBinaryMargin(const TextExample & exp, NormalizeType norm = UNNORMALIZED) const {
        std::vector<double> ret = GetScores(exp);
        if(ret.size() != 2) THROW_ERROR("Attempting to get binary score for non-binary classifier");
        return Normalize(ret[1]-ret[0], norm, exp.GetLength());
    }

    const Classifier::Learner GetLearner() const { return learner_; }
    void SetLearner(Classifier::Learner learner) { learner_ = learner; }

private:
    // The number of classes to divide things into
    int num_classes_;
    // The n-gram length of features
    int feature_n_;
    bool use_length_;
    // The weight to assign to keywords
    double keyword_weight_;
    // The alpha for a feature prior
    double feature_alpha_;
    // The alpha for a label prior
    double label_alpha_;
    double label_total_;
    std::vector<double> label_counts_;
    // The type of update rule to use for training
    Classifier::Learner learner_;
    // Weights to use
    std::vector<SparseMap> weights_;


};

}

#endif
