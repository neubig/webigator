#ifndef WEBIGATOR_SERVER_H__ 
#define WEBIGATOR_SERVER_H__

#include <iostream>
#include <fstream>
#include <webigator/config-webigator-server.h>
#include <webigator/data-store.h>
#include <webigator/text-classifier.h>
#include <webigator/task.h>
#include <xmlrpc-c/base.hpp>
#include <xmlrpc-c/registry.hpp>
#include <xmlrpc-c/server_abyss.hpp>
#include <boost/shared_ptr.hpp>
#include <tr1/unordered_map>

namespace webigator {

// A class to build features for the filterer
class WebigatorServer {
public:

    WebigatorServer() { }
    ~WebigatorServer() { }
    
    // Run the model
    void Run(const ConfigWebigatorServer & config);
    
    int MapId(int task) const {
        std::map<int,int>::const_iterator it = id_map_.find(task);
        if(it == id_map_.end()) THROW_ERROR("Could not find task " << task);
        return it->second;
    }

    const std::map<int,int> & GetIdMap() const { return id_map_; }
    std::map<int,int> & GetIdMap() { return id_map_; }
    const Task & GetTask(int task) const { return *tasks_[MapId(task)]; }
    Task & GetTask(int task) { return *tasks_[MapId(task)]; }
    const std::vector<boost::shared_ptr<Task> > & GetTasks() { return tasks_; }
    const DataStore & GetDataStore(int task) const { return GetTask(task).GetDataStore(); }
    DataStore & GetDataStore(int task) { return GetTask(task).GetDataStore(); }
    const TextClassifier & GetClassifier(int task) const { return GetTask(task).GetClassifier(); }
    TextClassifier & GetClassifier(int task) { return GetTask(task).GetClassifier(); }
    const boost::shared_ptr<xmlrpc_c::serverAbyss> & GetAbyssServer() const { return abyss_server_; }
    boost::shared_ptr<xmlrpc_c::serverAbyss> & GetAbyssServer() { return abyss_server_; }

    int AddTask(boost::shared_ptr<Task> task) {
        int task_id = -1;
        // Find a random task number that is not used yet
        do { task_id = rand(); } while(id_map_.find(task_id) != id_map_.end());
        id_map_[task_id] = tasks_.size();
        task->SetTaskId(task_id);
        tasks_.push_back(task);
        return task_id;
    }

private:

    boost::shared_ptr<xmlrpc_c::serverAbyss> abyss_server_;
    std::vector<boost::shared_ptr<Task> > tasks_;
    std::map<int,int> id_map_;

};

}

#endif

