#include "Common.h"
#include <iostream>
#include <assert.h>
using namespace std;

union JEndian
{
    short   i;
    char    arr[2];
};

int judgeEndian()
{
    JEndian je;
    je.i = 0x1234;
    if (je.arr[0] == 0x34 && je.arr[1] == 0x12)
    {
        cout << "it's little endian" << endl;
        return 0;
    }
    else if (je.arr[0] == 0x12 && je.arr[1] == 0x34)
    {
        cout << "it's big endian" << endl;
        return 1;
    }
    cout << "error" << endl;
    return -1;
}

unsigned long Htonl(unsigned long n)
{
    int ret = 0;
    int endian = judgeEndian();
    assert(endian != -1);
    if (endian == 0)   //Ğ¡¶Ë
    {
        ret = (n & 0x000000FF) << 24;
        ret |= (n & 0x0000FF00) << 8;
        ret |= (n & 0x00FF0000) >> 8;
        ret |= (n & 0xFF000000) >> 24;
        return ret;
    }
    return n;
}

unsigned long Ntohl(unsigned long n)
{
    int ret = 0;
    int endian = judgeEndian();
    assert(endian != -1);
    if (endian == 0)   //Ğ¡¶Ë
    {
        ret = (n & 0x000000FF) << 24;
        ret |= (n & 0x0000FF00) << 8;
        ret |= (n & 0x00FF0000) >> 8;
        ret |= (n & 0xFF000000) >> 24;
        return ret;
    }
    return n;
}

void endianTest()
{
    unsigned long n = 0x12345678;
    cout << hex << "n = " << n << endl;
    cout << "htonl, n = " << Htonl(n) << endl;
    cout << "ntohl, n = " << Ntohl(Htonl(n)) << endl;
}
