#ifndef CLASSIFIER_H__
#define CLASSIFIER_H__

namespace webigator {

class Classifier {

public:
    typedef enum {
        UNSPECIFIED,
        PERCEPTRON,
        NAIVE_BAYES,
        KEYWORD
    } Learner;

};

}

#endif
