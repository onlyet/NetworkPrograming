#include "pch.h"
//#include "Buffer.h"
//#include <algorithm>
//
//Buffer::Buffer() : 
//    buf_(NULL), b_(0), e_(0), cap_(0), exp_(512)
//{
//}
//
//Buffer::~Buffer()
//{
//    delete[] buf_;
//}
//
//void Buffer::clear()
//{
//    /*if (buf_ == nullptr)
//    LDebug("buf_ == nullptr");*/
//    delete[] buf_;
//    buf_ = NULL;
//    cap_ = 0;
//    b_ = e_ = 0;
//}
//
//size_t Buffer::size() const
//{
//    return e_ - b_;
//}
//
//bool Buffer::empty() const
//{
//    return e_ == b_;
//}
//
//char* Buffer::data() const
//{
//    return buf_ + b_;
//}
//
//char* Buffer::begin() const
//{
//    return buf_ + b_;
//}
//
//char* Buffer::end() const
//{
//    return buf_ + e_;
//}
//
//char* Buffer::makeRoom(size_t len)
//{
//    if (e_ + len <= cap_) {
//    }
//    else if (size() + len < cap_ / 2)
//        moveHead();
//    else
//        expand(len);
//
//    return end();
//}
//
//void Buffer::makeRoom()
//{
//    if (space() < exp_)
//        expand(0);
//}
//
//size_t Buffer::space() const
//{
//    return cap_ - e_;
//}
//
//void Buffer::addSize(size_t len)
//{
//    e_ += len;
//}
//
//char* Buffer::allocRoom(size_t len)
//{
//    char *p = makeRoom(len);
//    addSize(len);
//    return p;
//}
//
//Buffer& Buffer::append(const char* p, size_t len)
//{
//    memcpy(allocRoom(len), p, len);
//    return *this;
//}
//
//Buffer& Buffer::append(std::string s)
//{
//    return append(s.data(), s.size());
//}
//
//Buffer& Buffer::append(Slice slice)
//{
//    return append(slice.data(), slice.size());
//}
//
//Buffer& Buffer::append(const char* p)
//{
//    return append(p, strlen(p));
//}
//
//Buffer& Buffer::consume(size_t len)
//{
//    b_ += len;
//    if (size() == 0)
//        clear();
//    return *this;
//}
//
//Buffer& Buffer::absorb(Buffer& buf)
//{
//    if (&buf != this)
//    {
//        if (size() == 0)
//        {
//            char b[sizeof buf];
//            memcpy(b, this, sizeof b);
//            memcpy(this, &buf, sizeof b);
//            memcpy(&buf, b, sizeof b);
//            std::swap(exp_, buf.exp_);  // keep the origin exp_
//        }
//        else
//        {
//            append(buf.begin(), buf.size());
//            buf.clear();
//        }
//    }
//    return *this;
//}
//
//void Buffer::setSuggestSize(size_t sz)
//{
//    exp_ = sz;
//}
//
//Buffer::Buffer(const Buffer& b)
//{
//    copyFrom(b);
//}
//
//Buffer& Buffer::operator=(const Buffer& b)
//{
//    if (this == &b)
//        return *this;
//    delete[] buf_;
//    buf_ = NULL;
//    copyFrom(b);
//    return *this;
//}
//
//void Buffer::moveHead()
//{
//    //_DEPRECATE_UNCHECKED
//    std::copy(begin(), end(), buf_);
//    e_ -= b_; b_ = 0;
//}
//
//void Buffer::expand(size_t len)
//{
//    size_t ncap = std::max(exp_, std::max(2 * cap_, size() + len));
//    char *p = new char[ncap];
//    std::copy(begin(), end(), p);
//    e_ -= b_;
//    b_ = 0;
//    delete[] buf_;
//    buf_ = p;
//    cap_ = ncap;
//}
//
//void Buffer::copyFrom(const Buffer& b)
//{
//    memcpy(this, &b, sizeof b);
//    if (b.buf_)
//    {
//        buf_ = new char[cap_];
//        memcpy(data(), b.begin(), b.size());
//    }
//}
