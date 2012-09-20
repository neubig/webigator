#include <webigator/data-store.h>

using namespace std;
using namespace webigator;

void DataStore::AddNewExample(const TextExample & exp) {
    // Add this value to the cache
    if(!IsFinished(exp) && !InCache(exp)) {
        cache_.insert(exp);
        in_cache_.insert(exp.GetId());
    }
    // If the cache overflowed the maximum size, delete the worst candidate
    if((int)cache_.size() > max_cache_size_ && max_cache_size_ >= 0) {
        Cache::iterator it = --cache_.end();
        in_cache_.erase(it->GetId());
        cache_.erase(it);
    }
}
