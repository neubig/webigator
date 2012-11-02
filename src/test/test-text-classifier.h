#ifndef TEST_TEXT_CLASSIFIER_H__
#define TEST_TEXT_CLASSIFIER_H__

#include "test-base.h"
#include <boost/shared_ptr.hpp>
#include <webigator/text-classifier.h>
#include <webigator/text-example.h>

using namespace boost;

namespace webigator {

class TestTextClassifier : public TestBase {

public:

    TestTextClassifier() { }

    ~TestTextClassifier() { }

    int TestUpdateWithLabeledExamplePerceptron() {
        // Create a text example
        TextExample text_exp(0, "this is a test and a vest", 1);
        // Create a text classifier that is binary, uses unigram features,
        // and perceptron update 
        TextClassifier text_class(2, 1, Classifier::PERCEPTRON);
        text_class.UpdateWithLabeledExample(text_exp);
        text_class.UpdateWithLabeledExample(text_exp); // Doing this a second time should not update
        // Get the example of the weights
        SparseMap act_weights = text_class.GetBinaryWeights(),
                  exp_weights;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("this")))] = 1;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("is")))] = 1;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("a")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("test")))] = 1;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("and")))] = 1;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("vest")))] = 1;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("<LENGTH0>")))] = 1;
        return CheckAlmostMap(exp_weights, act_weights);
    }

    int TestUpdateWithLabeledExampleNaiveBayes() {
        // Create a text example
        TextExample text_exp(0, "this is a test and a vest", 1);
        // Create a text classifier that is binary, uses unigram features,
        // and perceptron update 
        TextClassifier text_class(2, 1, Classifier::NAIVE_BAYES);
        text_class.UpdateWithLabeledExample(text_exp);
        text_class.UpdateWithLabeledExample(text_exp); // Doing this a second time should still update
        // Get the example of the weights
        SparseMap act_weights = text_class.GetWeights(1),
                  exp_weights;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("this")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("is")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("a")))] = 4;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("test")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("and")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("vest")))] = 2;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("<LENGTH0>")))] = 2;
        return CheckAlmostMap(exp_weights, act_weights);
    }

    int TestUpdateWithLabeledKeywordPerceptron() {
        // Create a text example
        TextExample text_exp(0, "this is a test and a vest", 1);
        // Create a text classifier that is binary, uses unigram features,
        // and perceptron update 
        TextClassifier text_class(2, 1, Classifier::PERCEPTRON);
        text_class.UpdateWithLabeledExample(text_exp, Classifier::KEYWORD);
        text_class.UpdateWithLabeledExample(text_exp, Classifier::KEYWORD); // Doing this a second time for keywords should update
        // Get the example of the weights
        SparseMap act_weights = text_class.GetBinaryWeights(),
                  exp_weights;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("this")))] = 10;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("is")))] = 10;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("a")))] = 20;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("test")))] = 10;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("and")))] = 10;
        exp_weights[Dict::FeatID(GenericString<int>(1, Dict::WID("vest")))] = 10;
        return CheckMap(exp_weights, act_weights);
    }

    int TestCalcClassifierScorePerceptron() {
        // Set up the text classifier
        TextClassifier text_class(2, 1, Classifier::PERCEPTRON);
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("this")))] = 1;
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("is")))] = -1;
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("a")))] = 2;
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("test")))] = -2;
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("and")))] = 3;
        text_class.GetBinaryWeights()[Dict::FeatID(GenericString<int>(1, Dict::WID("vest")))] = -3;
        // Get the scores
        TextExample text_exp(0, "this is a pen");
        vector<double> exp_score(2, 2), act_score = text_class.GetScores(text_exp);
        exp_score[0] = 0;
        return CheckAlmostVector(exp_score, act_score);
    }

    int TestCalcClassifierScoreNaiveBayes() {
        // Set up the text classifier
        TextClassifier text_class(2, 1, Classifier::NAIVE_BAYES);
        text_class.SetUseLength(false);
        // This specifies the probability over labels
        text_class.SetLabelAlpha(2);
        text_class.SetLabelCount(0,1);
        text_class.SetLabelCount(1,2);
        // Add the features
        text_class.SetFeatureAlpha(1);
        text_class.GetWeights(1)[Dict::FeatID(GenericString<int>(1, Dict::WID("this")))] = 1;
        text_class.GetWeights(0)[Dict::FeatID(GenericString<int>(1, Dict::WID("is")))] =   1;
        text_class.GetWeights(1)[Dict::FeatID(GenericString<int>(1, Dict::WID("a")))] =    2;
        text_class.GetWeights(0)[Dict::FeatID(GenericString<int>(1, Dict::WID("test")))] = 2;
        text_class.GetWeights(1)[Dict::FeatID(GenericString<int>(1, Dict::WID("and")))] =  3;
        text_class.GetWeights(0)[Dict::FeatID(GenericString<int>(1, Dict::WID("vest")))] = 3;
        // Create the example
        TextExample text_example(0, "this is a pen");
        // First calculate the probability of the labels
        // p(l0) = (c(l0)+a/|L|)/(c+a)
        double pl0 = (1+2.0/2)/(3+2), pl1 = (2+2.0/2)/(3+2);
        // For each feature, the probability is the count plus the pseudo-count
        //          this            is              a               pen
        double p0 = (0+pl0)/(1+1) * (1+pl0)/(1+1) * (0+pl0)/(2+1) * (0+pl0)/(0+1);
        double p1 = (1+pl1)/(1+1) * (0+pl1)/(1+1) * (2+pl1)/(2+1) * (0+pl1)/(0+1);
        // Calculate the expected and actual scores
        vector<double> exp_score(2);
        exp_score[0] = log(p0/(p0+p1));
        exp_score[1] = log(p1/(p0+p1));
        vector<double> act_score = text_class.GetScores(text_example);
        return CheckAlmostVector(exp_score, act_score);
    }

    bool RunTest() {
        int done = 0, succeeded = 0;
        done++; cout << "TestUpdateWithLabeledExamplePerceptron()" << endl; if(TestUpdateWithLabeledExamplePerceptron()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestUpdateWithLabeledExampleNaiveBayes()" << endl; if(TestUpdateWithLabeledExampleNaiveBayes()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestUpdateWithLabeledKeywordPerceptron()" << endl; if(TestUpdateWithLabeledKeywordPerceptron()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalcClassfierScorePerceptron()" << endl; if(TestCalcClassifierScorePerceptron()) succeeded++; else cout << "FAILED!!!" << endl;
        done++; cout << "TestCalcClassfierScoreNaiveBayes()" << endl; if(TestCalcClassifierScoreNaiveBayes()) succeeded++; else cout << "FAILED!!!" << endl;
        cout << "#### TestTextClassifier Finished with "<<succeeded<<"/"<<done<<" tests succeeding ####"<<endl;
        return done == succeeded;
    }

private:

};

}

#endif
