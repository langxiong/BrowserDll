#include "stdafx.h"
#include "JSExternal.h"

JSExternal::JSExternal():
    m_lRefCount(0)
{

}

JSExternal::~JSExternal()
{
}

STDMETHODIMP JSExternal::QueryInterface(REFIID riid, void** ppv)
{
    if (ppv == NULL)
    {
        return E_INVALIDARG ;
    }

    *ppv = 0 ;
    if (IID_IUnknown == riid)
    {
        *ppv = (IUnknown*)this ;
    }
    else if (IID_IDispatch == riid)
    {
        *ppv = (IDispatch*)this ;
    }
    if (NULL != *ppv)
    {
        ((LPUNKNOWN)*ppv)->AddRef() ;
        return NOERROR ;
    }
    return E_NOINTERFACE ;
}

STDMETHODIMP_(ULONG) JSExternal::AddRef(void)
{
    assert(m_lRefCount >= 0) ;
    return (ULONG)::InterlockedIncrement(&m_lRefCount) ;
}

STDMETHODIMP_(ULONG) JSExternal::Release(void)
{
    assert(m_lRefCount > 0) ;
    if (0 == ::InterlockedDecrement(&m_lRefCount))
    {
        delete this ;
        return 0L ;
    }
    return (ULONG)m_lRefCount ;
}

STDMETHODIMP JSExternal::GetTypeInfoCount(UINT*)
{
    return E_NOTIMPL ;
}

STDMETHODIMP JSExternal::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL ;
}

STDMETHODIMP JSExternal::GetIDsOfNames(REFIID, LPOLESTR* names, UINT namesCount, LCID, DISPID* identify)
{
    if ( (names == NULL) || (identify == NULL) )
    {
        return E_INVALIDARG ;
    }
    HRESULT hresult = NOERROR ;
    for (UINT i = 0; i < namesCount; i++) 
    {
        bool find = false ;
        LPOLESTR bstrValue = names[i] ;
        MyString funcName;
        if(bstrValue != NULL)
        {
            funcName = bstrValue ;
        }


        auto it = std::find_if(m_externalItems.cbegin(), m_externalItems.cend(), [funcName](const TExternalItem& item) {
            return item.m_name == funcName;
        });

        if (it == m_externalItems.cend())
        {
            hresult = ResultFromScode(DISP_E_UNKNOWNNAME);
            identify[i] = DISPID_UNKNOWN;
        }
        else
        {
            identify[i] = (DISPID)(std::distance(m_externalItems.cbegin(), it) + 1);
        }
    }
    return hresult ;
}

STDMETHODIMP JSExternal::Invoke(DISPID identify, REFIID, LCID, WORD flags, DISPPARAMS* params, VARIANT* result, EXCEPINFO*, UINT*)
{
    if (!(flags & DISPATCH_METHOD))
    {
        return S_OK ;
    }

    if ((identify > 0) && (identify <= (DISPID)m_externalItems.size()))
    {
        (m_externalItems[identify - 1].m_pFnRun)(params, result) ;
        //延后清理资源
        if ( (result != NULL) && (result->vt == VT_BSTR) )
        {
            m_invokeResult.Empty() ;
            m_invokeResult = result->bstrVal ;
        }        
    }
    return S_OK ;
}
