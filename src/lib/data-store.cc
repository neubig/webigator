#include <webigator/data-store.h>

using namespace std;
using namespace boost;
using namespace webigator;

void DataStore::AddNewExample(const TextExample & exp) {
    // If we are finished, return
    if(IsFinished(exp)) return;
    // Check if the value is in the cache
    CacheMap::const_iterator it = in_cache_.find(exp.GetId());
    // Add it to the cache if it isn't
    if(it == in_cache_.end()) {
        shared_ptr<TextExample> ptr(new TextExample(exp));
        cache_.insert(ptr);
        in_cache_.insert(MakePair(exp.GetId(), ptr));
        // If the cache overflowed the maximum size, delete the worst candidate
        if((int)cache_.size() > max_cache_size_ && max_cache_size_ >= 0) {
            Cache::iterator it = --cache_.end();
            in_cache_.erase((*it)->GetId());
            cache_.erase(it);
        }
    }
    // Update the score if it is in the cache, but the score is different
    else if(it->second->GetScore() != exp.GetScore()) {
        cache_.erase(it->second);
        shared_ptr<TextExample> ptr(new TextExample(exp));
        cache_.insert(ptr);
        in_cache_.insert(MakePair(exp.GetId(), ptr));
    }
}

TextExample DataStore::PopNextExample(const TextClassifier & classifier) {
    if(cache_.size() == 0)
        THROW_ERROR("Attempting to pop from an empty cache in DataStore");
    TextExample ret;
    while(true) {
        // Pop the top
        ret = **cache_.begin();
        cache_.erase(cache_.begin());
        // Update the score
        ret.SetScore(classifier.GetBinaryMargin(ret));
        // If this is still the best after score updating, OK, otherwise re-insert it and try again
        if(cache_.size() == 0 || (*cache_.begin())->GetScore() <= ret.GetScore())
            break;
        cache_.insert(shared_ptr<TextExample>(new TextExample(ret)));
    }
    in_cache_.erase(ret.GetId());
    return ret;
}
