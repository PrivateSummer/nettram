#pragma once

namespace nt
{
// 功能： 退出作用域时自动执行一个函数

template<typename ObjT, typename Data1T>
struct ScopedDestructor
{
    typedef void (ObjT::*FuncT)(Data1T);

    ScopedDestructor(FuncT sendFunc, ObjT *obj, Data1T data1)
        : m_sendFunc(sendFunc), m_obj(obj), m_data1(data1), m_cancel(false) {}

    void Cancel()
    {
        m_cancel = true;
    }
    ~ScopedDestructor()
    {
        if(!m_cancel)
        {
            (m_obj->*m_sendFunc)(m_data1);
        }
    }
private:
    FuncT m_sendFunc;
    ObjT *m_obj;
    Data1T m_data1;
    bool m_cancel;
};

template<typename Data1T>
struct ScopedDestructorFunc
{
    typedef void (*FuncT)(Data1T);

    ScopedDestructorFunc(FuncT sendFunc, Data1T data1)
        : m_sendFunc(sendFunc), m_data1(data1), m_cancel(false) {}

    void Cancel()
    {
        m_cancel = true;
    }
    ~ScopedDestructorFunc()
    {
        if(!m_cancel)
        {
            (*m_sendFunc)(m_data1);
        }
    }
private:
    FuncT m_sendFunc;
    Data1T m_data1;
    bool m_cancel;
};

}; //namespace nt
