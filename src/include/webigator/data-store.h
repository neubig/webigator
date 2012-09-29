#ifndef DATA_STORE_H__
#define DATA_STORE_H__

#include <set>
#include <tr1/unordered_set>
#include <webigator/text-example.h>
#include <webigator/text-classifier.h>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

namespace webigator {

class DataStore {

public:

    typedef std::set<boost::shared_ptr<TextExample>, TextExampleScoreMore> Cache;
    typedef std::tr1::unordered_map<ExampleId, boost::shared_ptr<TextExample> > CacheMap;

    DataStore() : max_cache_size_(-1), uniq_(true) { }
    ~DataStore() { }
    
    void SetFinished(const TextExample & exp) {
        finished_.insert(exp.GetId());
    }
    bool IsFinished(const TextExample & exp) {
        return finished_.find(exp.GetId()) != finished_.end();
    }
    bool InCache(const TextExample & exp) {
        return in_cache_.find(exp.GetId()) != in_cache_.end();
    }

    void AddNewExample(const TextExample & exp);

    const TextExample & PeekNextExample() {
        if(cache_.size() == 0)
            THROW_ERROR("Attempting to peek at an empty cache in DataStore");
        return **cache_.begin();
    }
    TextExample PopNextExample(const TextClassifier & classifier);

    void RescoreCache(const TextClassifier & classifier) {
        Cache old_cache = cache_;
        cache_.clear();
        BOOST_FOREACH(boost::shared_ptr<TextExample> exp, old_cache) {
            exp->SetScore(classifier.GetBinaryMargin(*exp));
            cache_.insert(exp);
        }        
    }

    const Cache & GetCache() const { return cache_; }
    Cache & GetCache() { return cache_; }
    int GetCacheSize() const { return cache_.size(); }
    int GetMaxCacheSize() const { return max_cache_size_; }
    void SetMaxCacheSize(int max_cache_size) { max_cache_size_ = max_cache_size; }
    bool GetUniq() const { return uniq_; }
    void SetUniq(bool uniq) { uniq_ = uniq; }
    const std::set<std::string> & GetKeywords() const { return keywords_; }
    std::set<std::string> & GetKeywords() { return keywords_; }
    const void AddKeyword(const std::string & str) { keywords_.insert(str); }

private:
    int max_cache_size_;
    bool uniq_;
    Cache cache_;
    CacheMap in_cache_;
    std::set<ExampleId> finished_;
    std::tr1::unordered_set<GenericString<int>, GenericHash<GenericString<int> > > popped_strs_;
    std::set<std::string> keywords_;

};

}

#endif
