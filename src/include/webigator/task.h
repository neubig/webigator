#ifndef TASK_H__
#define TASK_H__

#include <webigator/text-classifier.h>
#include <webigator/data-store.h>

namespace webigator {

class Task {

public:

    Task() { }

    int GetTaskId() const { return task_id_; }
    void SetTaskId(int task_id) { task_id_ = task_id; }
    const DataStore & GetDataStore() const { return data_store_; }
    DataStore & GetDataStore() { return data_store_; }
    const TextClassifier & GetClassifier() const { return classifier_; }
    TextClassifier & GetClassifier() { return classifier_; }
    void SetClassifier(const TextClassifier & classifier) { classifier_ = classifier; }
    bool HasUserPassword() const { return user_password.length() != 0; }
    void SetUserPassword(std::string str)  { user_password = str; }

protected:

    int task_id_;
    std::string user_password;
    std::string admin_password;
    DataStore data_store_;
    TextClassifier classifier_;

};

}

#endif
