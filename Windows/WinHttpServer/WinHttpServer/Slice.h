#ifndef __SLICE_H__
#define __SLICE_H__

#include <string>
#include <vector>

//Slice作为Buffer的扩展，只维护Buffer的指针，不具有自己的内存
//所以要确保Slice在Buffer的生命周期内使用
class Slice
{
public:
    Slice();
    Slice(const char* b, const char* e);
    Slice(const char* d, size_t n);
    Slice(const std::string& s);
    Slice(const char* s);

    const char* data() const;
    const char* begin() const;
    const char* end() const;

    char front();
    char back();

    size_t size() const;
    bool empty() const;

    void resize(size_t sz);
    void clear();

    //bool operator<(const Slice& rhs);
    //bool operator==(const Slice& rhs);
    //bool operator!=(const Slice& rhs);

    //return the eated data
    //eat后，原来的Slice首元素指向空格或\r
    Slice eatWord();
    //eat后，原来的Slice首元素指向\r
    Slice eatLine();
    Slice eat(size_t sz);
    Slice sub(int boff, int eoff = 0) const;
    //跳过首尾空格
    Slice& trimSpace();

    char operator[](size_t n) const;

    std::string toString() const;
    operator std::string() const;

    // Three-way comparison.  Returns value:
    int compare(const Slice& b) const;

    // Return true if "x" is a prefix of "*this"
    bool starts_with(const Slice& x) const;
    bool end_with(const Slice& x) const;

    std::vector<Slice> split(char ch) const;

private:
    const char* m_pb;   //首元素
    const char* m_pe;   //尾后元素
};

bool operator<(const Slice& x, const Slice& y);

bool operator==(const Slice& x, const Slice& y);

bool operator!=(const Slice& x, const Slice& y);

#endif // !__SLICE_H__
