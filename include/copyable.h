#ifndef COPYABLE_H
#define COPYABLE_H

namespace mymuduo
{
    class copyable
    {
        /* 派生类允许拷贝或者等号赋值 */
    protected:
        copyable() = default;
        ~copyable() = default;
    };
}


#endif