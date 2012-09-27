
#ifndef WEBIGATOR_GENERIC_STRING_H__
#define WEBIGATOR_GENERIC_STRING_H__

#include <list>
#include <stdexcept>
#include <cstring>
#include <sstream>
#include <vector>
#include <iostream>

// will boundary check when defined
// #define GENERIC_STRING_SAFE

namespace webigator {

class StringUtil;

// an implementation of a string, kept in memory
template <class GenericChar>
class GenericStringImpl {

public:
    int length_;
    int count_;
    GenericChar* chars_;

    GenericStringImpl() : length_(0), count_(1), chars_(0) { }
    GenericStringImpl(const std::string & str) :
        length_(str.length()/sizeof(GenericChar)), count_(1) {
        chars_ = new GenericChar[length_];
        memcpy(chars_, &str[0], sizeof(GenericChar)*length_);
    }
    GenericStringImpl(int length, GenericChar def = 0) : length_(length), count_(1) {
        chars_ = new GenericChar[length];
        for(int i = 0; i < length; i++)
            chars_[i] = def;
    }
    GenericStringImpl(const GenericStringImpl & impl) : length_(impl.length_), count_(1) {
        chars_ = new GenericChar[length_];
        memcpy(chars_, impl.chars_, sizeof(GenericChar)*length_);
    }
    ~GenericStringImpl() {
        if(chars_)
            delete [] chars_;
    }

    int dec() { return --count_; }
    int inc() { return ++count_; }

};

template <class GenericChar>
class GenericString {

public:
    friend class StringUtil;
    
private:
    GenericStringImpl<GenericChar>* impl_;

public:

    typedef std::list<GenericString> Tokens;

    // ctor
    GenericString() : impl_(0) { }
    GenericString(const GenericString & str) { 
        impl_ = str.impl_;
        if(impl_) impl_->inc();
    }
    GenericString(int length, GenericChar def = 0) { impl_ = new GenericStringImpl<GenericChar>(length, def); }
    GenericString(const std::vector<GenericChar> & vec) { 
        impl_ = new GenericStringImpl<GenericChar>(vec.size());
        for(int i = 0; i < (int)vec.size(); i++)
            impl_->chars_[i] = vec[i]; 
    }
    GenericString(const std::string & str) { 
        impl_ = new GenericStringImpl<GenericChar>(str);
    }
    
    // dtor
    ~GenericString() {
        if(impl_ && !impl_->dec())
            delete impl_;
    }

    // Get the representation of the memory as a string
    std::string str() const {
        if(!impl_) return std::string();
        int len = sizeof(GenericChar)*impl_->length_;
        std::string ret(len, (char)0);
        memcpy(&ret[0], impl_->chars_, len);
        return ret;
    }

    // tokenize the string using the characters in the delimiter string
    Tokens tokenize(const GenericString & delim, bool includeDelim = false) const {
        int i,j,s=0;
        const int l=length(),dl=delim.length();
        std::list<GenericString> ret;
        for(i = 0; i < l; i++) {
            for(j = 0; j < dl && delim[j] != impl_->chars_[i]; j++);
            if(j != dl) {
                if(s != i)
                    ret.push_back(substr(s,i-s));
                if(includeDelim)
                    ret.push_back(substr(i,1));
                s = i+1;
            }
        }
        if(s != i)
            ret.push_back(substr(s,i-s));
        return ret;
    }

    // splice a string into the appropriate location
    inline void splice(const GenericString& str, int pos) {
        const int l = str.length();
        if(!l) 
            return;
#ifdef GENERIC_STRING_SAFE
        if(pos+l > length())
            throw std::runtime_error("GenericString splice index out of bounds");
#endif
        memcpy(impl_->chars_+pos, str.getImpl()->chars_, sizeof(GenericChar)*l);
    }

    GenericString substr(int s) const {
        const int l = length()-s;
#ifdef GENERIC_STRING_SAFE
        if(s+l > length())
            throw std::runtime_error("GenericString substr index out of bounds");
#endif
        GenericString ret(l);
        memcpy(ret.getImpl()->chars_, impl_->chars_+s, sizeof(GenericChar)*l);
        return ret;
    }
    

    GenericString substr(int s, int l) const {
#ifdef GENERIC_STRING_SAFE
        if(s+l > length()) {
            std::ostringstream buff;
            buff << "substr out of bounds (length="<<length()<<", s="<<s<<", l="<<l<<")";
            throw std::runtime_error(buff.str());
        }
#endif
        GenericString ret(l);
        memcpy(ret.getImpl()->chars_, impl_->chars_+s, sizeof(GenericChar)*l);
        return ret;
    }
    
    inline GenericChar & operator[](int i) {
#ifdef GENERIC_STRING_SAFE
        if(impl_ == 0 || i < 0 || (int)i >= impl_->length_)
            throw std::runtime_error("string index out of bounds");
#endif
        return getImpl()->chars_[i];
    }
    
    inline const GenericChar & operator[](int i) const {
#ifdef GENERIC_STRING_SAFE
        if(impl_ == 0 || i < 0 || (int)i >= impl_->length_)
            throw std::runtime_error("string index out of bounds");
#endif
        return impl_->chars_[i];
    }
    
    GenericString & operator= (const GenericString &str) {
        if(impl_ && !impl_->dec())
            delete impl_;
        impl_ = str.impl_;
        if(impl_) impl_->inc();
        return *this;
    }


    inline int length() const {
        return (impl_?impl_->length_:0); 
    }


    inline size_t getHash() const {
        size_t hash = 5381;
        if(impl_==0)
            return hash;
        const int l = impl_->length_;
        const GenericChar* cs = impl_->chars_;
        for(int i = 0; i < l; i++)
            hash = ((hash << 5) + hash) + cs[i]; /* hash * 33 + x[i] */
        return hash;
    }

    const GenericStringImpl<GenericChar> * getImpl() const {
        return impl_;
    }
    
    GenericStringImpl<GenericChar> * getImpl() {
        if(impl_->count_ != 1) {
            impl_->dec();
            impl_ = new GenericStringImpl<GenericChar>(*impl_);
        }
        return impl_;
    }

};

template <class GenericChar>
inline GenericString<GenericChar> operator+(const GenericString<GenericChar>& a, const GenericChar& b) {
    const GenericStringImpl<GenericChar> * aimp = a.getImpl();
    if(aimp == 0) {
        GenericString<GenericChar> ret(1);
        ret[0] = b;
        return ret;
    }
    GenericString<GenericChar> ret(aimp->length_+1);
    ret.splice(a,0);
    ret[aimp->length_]=b;
    return ret;
}

template <class GenericChar>
inline GenericString<GenericChar> operator+(const GenericString<GenericChar>& a, const GenericString<GenericChar>& b) {
    const GenericStringImpl<GenericChar> * aimp = a.getImpl();
    if(aimp == 0)
        return b;
    const GenericStringImpl<GenericChar> * bimp = b.getImpl();
    if(bimp == 0)
        return a;
    GenericString<GenericChar> ret(aimp->length_+bimp->length_);
    ret.splice(a,0);
    ret.splice(b,aimp->length_);
    return ret;
}

template <class GenericChar>
inline std::ostream & operator<<(std::ostream & out, const GenericString<GenericChar>& a) {
    out << "[";
    for(int i = 0; i < (int)a.length(); i++) {
        if(i != 0) out << ", ";
        out << a[i];
    }
    out << "]";
    return out;
}

template <class GenericChar>
inline bool operator<(const GenericString<GenericChar> & a, const GenericString<GenericChar> & b) {
    int i;
    const int al = a.length(), bl = b.length(), ml=std::min(al,bl);
    for(i = 0; i < ml; i++) {
        if(a[i] < b[i]) return true;
        else if(b[i] < a[i]) return false;
    }
    return (bl != i);
}

template <class GenericChar>
inline bool operator==(const GenericString<GenericChar> & a, const GenericString<GenericChar> & b) {
    int i;
    const int al = a.length();
    if(al!=b.length())
        return false;
    for(i = 0; i < al; i++)
        if(a[i] != b[i]) return false;
    return true;
}

template <class GenericChar>
inline bool operator!=(const GenericString<GenericChar> & a, const GenericString<GenericChar> & b) {
    return !(a==b);
}

template <class T>
class GenericHash {
public:
    size_t operator()(const T & x) const {
        return x.getHash();
    }
};

}

#endif
