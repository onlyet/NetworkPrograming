//#ifndef __BUFFER_H__
//#define __BUFFER_H__
//
//#include "slice.h"
//#include <string>
//
////std::copy : _DEPRECATE_UNCHECKED：在Preprocessor添加_SCL_SECURE_NO_WARNINGS
//
//class Buffer 
//{
//public:
//    Buffer();
//    ~Buffer();
//    Buffer(const Buffer& b);
//    Buffer& operator=(const Buffer& b);
//
//    void clear();
//    size_t size() const;
//    bool empty() const;
//    char* data() const;
//    char* begin() const;
//    char* end() const;
//    char* makeRoom(size_t len);
//    void makeRoom();
//    size_t space() const;	//缓存可用空间
//    void addSize(size_t len);
//    char* allocRoom(size_t len);
//
//    Buffer& append(const char* p, size_t len);
//    Buffer& append(std::string s);
//    Buffer& append(Slice slice);
//    Buffer& append(const char* p);
//    template <class T>
//    Buffer& appendValue(const T& v) 
//    {
//        append((const char *)&v, sizeof v);
//        return *this;
//    }
//
//    Buffer& consume(size_t len);
//    Buffer& absorb(Buffer& buf);
//    void setSuggestSize(size_t sz);
//
//    //转换为Slice
//    operator Slice() { return Slice(data(), size()); }
//
//private:
//    //将未读数据移动到头部
//    void moveHead();
//    void expand(size_t len);	//增加buffer长度，类似于vector
//    void copyFrom(const Buffer& b);
//
//private:
//    char* buf_;
//    size_t b_;      //未读开始位置
//    size_t e_;      //未读结束位置
//    size_t cap_;    //当前缓冲区容量
//    size_t exp_;
//};
//
//#endif // !__BUFFER_H__
//
//
//// 0 <= b <= e <= cap
