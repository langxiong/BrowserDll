#pragma once

#pragma warning(push, 3)
#include <atlbase.h>
#include <mshtml.h>
#include <exdisp.h>
#include <comutil.h>
#pragma warning(pop)

#include <vector>
#include <memory>

namespace MyWeb
{
    struct TExternalItem;

    /** JAVASCRIPT通信
    */
    class JSExternal :
        public IDispatch
    {
    public:

        /** 构造函数
        */
        JSExternal();

        /** 析构函数
        */
        virtual ~JSExternal();

        void AddExternalItem(const std::shared_ptr<TExternalItem>& spExternalItem);

        void RemoveExternalItem(const MyString& methodName);
    public:

        /** IUnknown
        */
        STDMETHODIMP QueryInterface(REFIID, void**);
        STDMETHODIMP_(ULONG) AddRef(void);
        STDMETHODIMP_(ULONG) Release(void);

    public:

        /** IDispatch
        */
        STDMETHODIMP GetTypeInfoCount(UINT*);
        STDMETHODIMP GetTypeInfo(UINT, LCID, ITypeInfo**);
        STDMETHODIMP GetIDsOfNames(REFIID, LPOLESTR* names, UINT namesCount, LCID, DISPID* identify);
        STDMETHODIMP Invoke(DISPID identify, REFIID, LCID, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO*, UINT*);

    public:

        /** 返回值
        */
        CComBSTR m_invokeResult;

    private:

        std::vector<std::shared_ptr<TExternalItem>> m_spExternalItems;

        /** 引用计数
        */
        volatile LONG m_lRefCount;
    };
}