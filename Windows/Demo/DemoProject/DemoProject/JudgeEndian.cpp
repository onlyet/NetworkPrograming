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
    int ret = judgeEndian();
    assert(ret != -1);
    if (ret == 1)   //Ð¡¶Ë
    {

    }
    else
    {

    }
}