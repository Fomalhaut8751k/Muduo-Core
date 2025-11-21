#ifndef NONCOPYABLE_H
#define NONCOPYABLE_H


class noncopyable
{
public:
    /*
        派生类的拷贝构造的时候需要先调用基类的拷贝构造
        但是基类的拷贝构造被删除了，因此派生类也无法拷贝构造
        等号赋值也一样。
    */
    noncopyable(const noncopyable&) = delete;
    noncopyable& operator=(const noncopyable&) = delete;

protected:
    /*
        派生类的构造函数调用时会先调用基类的构造函数
        私有继承下，派生类依然可以范围基类保护权限下的方法
    */
    noncopyable() = default;
    ~noncopyable() = default;
};


#endif