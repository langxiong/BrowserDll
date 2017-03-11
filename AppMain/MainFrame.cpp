#include "stdafx.h"
#include "MainFrame.h"
#include "AppCommon.h"

#include "Poco/FileStream.h"
#include "Poco/StreamCopier.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Parser.h"
#include "Poco/Nullable.h"
#include "Poco/Dynamic/Struct.h"

#include "StringHelper.h"
#include <vector>
#include <unordered_map>
#include <ppl.h>
#include <random>
#include <array>
#include "MyListHeaderItem.h"
#include "MyListElementItem.h"
#include "MyListContainer.h"
#include "MyListSubContainer.h"
#include "UISubsystem.h"
#include "resource.h"

#include "FindUtil.h"
#include "DateTimeUtil.h"

#include "Poco/File.h"

using namespace DuiLib;
using namespace Poco;
using namespace Poco::Util;
using namespace Poco::JSON;
using namespace Poco::Dynamic;

namespace MyWeb {
    using namespace MyWeb;
	using namespace UI_MSGS;
	using namespace Constants;
    namespace UI {
        struct ItemData
        {
            std::wstring _productName;
            std::wstring _platformName;
            std::wstring _productReturn;
            std::wstring _intestmentCircle;
        };

        CMainFrame::CMainFrame(uint32_t frameID) :
            m_frameID(frameID),
            m_columns(5),
            m_tableTexts(5),
            m_gapItemDatas(5),
            m_gapItemDatasUpdate(5),
            m_pListTable(nullptr),
            m_pListMain(nullptr),
            m_pBrowserEChart(nullptr),
            m_pLblTitle(nullptr),
            m_pLblStatus(nullptr),
            m_pVolume(new CVolumeUI),
            m_pComboDate(new CComboUI),
            m_pCombo0(nullptr),
            m_pCombo1(nullptr),
            m_pCombo2(nullptr),
            m_pCombo3(nullptr),
            m_pCombo4(nullptr),
            m_pLbl0(nullptr),
            m_pLbl1(nullptr),
            m_pLbl2(nullptr),
            m_pLbl3(nullptr),
            m_pLbl4(nullptr),
            m_pHor0(nullptr),
            m_pHor1(nullptr),
            m_pHor2(nullptr),
            m_pHor3(nullptr),
            m_pHor4(nullptr)
        {
        }
        CMainFrame::~CMainFrame()
        {
        }
        void CMainFrame::InitWindow()
        {
            m_pTabMain = static_cast<CTabLayoutUI*>(m_PaintManager.FindControl(_T("tabMain")));

            m_pListTable = static_cast<CListUI*>(m_PaintManager.FindControl(_T("listTable")));
            m_pListMain = static_cast<CListUI*>(m_PaintManager.FindControl(_T("listMain")));
            m_pBrowserEChart = static_cast<CNewWebBrowserUI*>(m_PaintManager.FindControl(_T("browserEChart")));

            m_pLblTitle = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lblTitle")));
            m_pLblStatus = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lblStatus")));
            m_pComboDate = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("comboDate")));

            m_pCombo0 = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo0")));
            m_pCombo1 = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo1")));
            m_pCombo2 = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo2")));
            m_pCombo3 = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo3")));
            m_pCombo4 = static_cast<CComboUI*>(m_PaintManager.FindControl(_T("combo4")));

            m_pLbl0 = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl0")));
            m_pLbl1 = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl1")));
            m_pLbl2 = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl2")));
            m_pLbl3 = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl3")));
            m_pLbl4 = static_cast<CLabelUI*>(m_PaintManager.FindControl(_T("lbl4")));

            m_pHor0 = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor0")));
            m_pHor1 = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor1")));
            m_pHor2 = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor2")));
            m_pHor3 = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor3")));
            m_pHor4 = static_cast<CHorizontalLayoutUI*>(m_PaintManager.FindControl(_T("hor4")));

            m_pHor0->EnableScrollBar(false, true);
            m_pHor1->EnableScrollBar(false, true);
            m_pHor2->EnableScrollBar(false, true);
            m_pHor3->EnableScrollBar(false, true);
            m_pHor4->EnableScrollBar(false, true);

            CContainerUI* pStatisticsContainer = static_cast<CContainerUI*>(m_PaintManager.FindControl(_T("statisticsContainer")));
            // pStatisticsContainer->Add(m_pVolume);
            assert(m_pListTable && m_pListMain && m_pLblTitle && m_pLblStatus && m_pComboDate);
            InitListTable();
            InitList();
            CenterWindow();
            RunDataInit();
        }
        void CMainFrame::OnFinalMessage(HWND hWnd)
        {
            m_cts.cancel();
            Sleep(500);
            WindowImplBase::OnFinalMessage(hWnd);
        }
        void CMainFrame::Notify(DuiLib::TNotifyUI & msg)
        {
            if (msg.sType == DUI_MSGTYPE_TIMER)
            {
                if (msg.wParam == TIMER_UPDATE_CAIPIAO_DATA_ID)
                {
                    m_PaintManager.KillTimer(msg.pSender, TIMER_UPDATE_CAIPIAO_DATA_ID);
                    RunDataUpdate();
                }
                return;
            }

            if (msg.sType == DUI_MSGTYPE_CLICK)
            {
                if (msg.pSender->GetName() == _T("btnClose"))
                {
                    Close();
                }
                if (msg.pSender->GetName() == _T("btnMin"))
                {
                    SendMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
                }
                else if (msg.pSender->GetName() == _T("btnNew"))
                {
                    PostMessage(WM_USER_CREATE_FRAME);
                }
                return;
            }

            if (msg.sType == DUI_MSGTYPE_SELECTCHANGED)
            {
                if (msg.pSender->GetName() == L"optViewNormal")
                {
                    m_pTabMain->SelectItem(1);
                }
                else if (msg.pSender->GetName() == L"optViewGraph")
                {
                    m_pTabMain->SelectItem(0);
                }
                return;
            }

            if (msg.sType == DUI_MSGTYPE_ITEMSELECT)
            {
                if (msg.pSender == m_pComboDate)
                {
                    OnSelectComboDate(msg.wParam);
                }
                else if (msg.pSender == m_pCombo0)
                {
                    OnSelectCombo0(msg.wParam);
                }
                else if (msg.pSender == m_pCombo1)
                {
                    OnSelectCombo1(msg.wParam);
                }
                else if (msg.pSender == m_pCombo2)
                {
                    OnSelectCombo2(msg.wParam);
                }
                else if (msg.pSender == m_pCombo3)
                {
                    OnSelectCombo3(msg.wParam);
                }
                else if (msg.pSender == m_pCombo4)
                {
                    OnSelectCombo4(msg.wParam);
                }
                return;
            }
            if (msg.sType == UI_MSG_HIGHT_LIGHT_ITEM)
            {
                if (msg.pSender->GetInterface(CMyListElementItem::DUI_CTR_MY_LIST_ELEMENT_ITEM.c_str()))
                {
                    CControlUI* pCtrl = msg.pSender->GetParent();
                    if (!pCtrl->GetInterface(CMyListSubContainer::DUI_CTR_MY_LIST_SUB_CONTAINER.c_str()))
                    {
                        return;
                    }

                    auto getDataPos = [&]() -> std::pair<int, int>
                    {
                        auto pListContainer = static_cast<CMyListContainer*>(
                            pCtrl->GetParent()->GetInterface(CMyListContainer::DUI_CTR_MY_LIST_CONTAINER.c_str()));

                        return std::make_pair(m_pListMain->GetItemIndex(pListContainer), pListContainer->GetItemIndex(pCtrl) - 1);
                    };

                    auto dataPos = getDataPos();
                    OnHightlightDataByMouse(dataPos.first, dataPos.second, true);
                }
                return;
            }

            if (msg.sType == UI_MSG_UNHIGHT_LIGHT_ITEM)
            {
                if (msg.pSender->GetInterface(CMyListElementItem::DUI_CTR_MY_LIST_ELEMENT_ITEM.c_str()))
                {
                    CControlUI* pCtrl = msg.pSender->GetParent();
                    if (!pCtrl->GetInterface(CMyListSubContainer::DUI_CTR_MY_LIST_SUB_CONTAINER.c_str()))
                    {
                        return;
                    }

                    auto getDataPos = [&]() -> std::pair<int, int>
                    {
                        auto pListContainer = static_cast<CMyListContainer*>(
                            pCtrl->GetParent()->GetInterface(CMyListContainer::DUI_CTR_MY_LIST_CONTAINER.c_str()));

                        return std::make_pair(m_pListMain->GetItemIndex(pListContainer), pListContainer->GetItemIndex(pCtrl) - 1);
                    };

                    auto dataPos = getDataPos();
                    OnHightlightDataByMouse(dataPos.first, dataPos.second, false);
                }
                return;
            }

            if (msg.sType == DUI_MSGTYPE_RETURN || DUI_MSGTYPE_TEXTCHANGED)
            {
                if (msg.pSender->GetInterface(DUI_CTR_EDIT))
                {
                    OnEditReturn(msg.pSender->GetName().GetData(), msg.pSender->GetText().GetData());
                }
                return;
            }
        }
        LRESULT CMainFrame::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL & bHandled)
        {
            bHandled = TRUE;
            if (uMsg == WM_USER_CREATE_FRAME)
            {
                OnCreateFrame();
                return S_OK;
            }
            else if (uMsg == WM_USER_ADD_CAIPIAO_DATA)
            {
                if (lParam)
                {
                    std::wstring* pStrStatusText = reinterpret_cast<std::wstring*>(lParam);
                    UpdateStatusText(*pStrStatusText);
                    delete pStrStatusText;
                }
                TCaipiaoData* pData = reinterpret_cast<TCaipiaoData*>(wParam);
                OnAddCaipiaoData(*pData);
                // m_pVolume->Invalidate();
                delete pData;
            }
            else if (uMsg == WM_USER_UPDATE_CAIPIAO_DATA)
            {
                if (lParam)
                {
                    std::wstring* pStrStatusText = reinterpret_cast<std::wstring*>(lParam);
                    UpdateStatusText(*pStrStatusText);
                    delete pStrStatusText;
                }
                TCaipiaoData* pData = reinterpret_cast<TCaipiaoData*>(wParam);
                OnUpdateCaipiaoData(*pData);
                // m_pVolume->Invalidate();
                delete pData;
            }
            else if (uMsg == WM_USER_LOAD_CAIPIAO_DATA_FINISH)
            {
                m_PaintManager.SetTimer(m_PaintManager.GetRoot(), TIMER_UPDATE_CAIPIAO_DATA_ID, 
                    config().getInt(key_nTimerUpdateUserData, 5) * 1000 * 60);
                return S_OK;
            }
            else if (uMsg == WM_USER_UPDATE_CAIPIAO_DATA_FINISH)
            {
                std::swap(m_gapItemDatas, m_gapItemDatasUpdate);
                std::swap(m_allTheDatas, m_allTheDatasUpdate);

                m_pListTable->RemoveAll();
                // 为ListTable数据添加同步添加更多的显示元素
                for (auto& vDataByDate: m_allTheDatas)
                {
                    for (auto& vDataByLevel: vDataByDate.second)
                    {
                        CListTextElementUI* pListText = new CListTextElementUI;
                        m_pListTable->Add(pListText);
                    }
                }

                m_PaintManager.SetTimer(m_PaintManager.GetRoot(), TIMER_UPDATE_CAIPIAO_DATA_ID, 
                    config().getInt(key_nTimerUpdateUserData, 5) * 1000 * 60);
                return S_OK;
            }
            else if (uMsg == WM_USER_UPDATE_STATUS_TEXT)
            {
                if (wParam)
                {
                    std::wstring* pStr = reinterpret_cast<std::wstring*>(wParam);
                    UpdateStatusText(*pStr);
                    delete pStr;
                }
            }
            bHandled = FALSE;
            return WindowImplBase::HandleCustomMessage(uMsg, wParam, lParam, bHandled);
        }
        UILIB_RESOURCETYPE CMainFrame::GetResourceType() const
        {

#ifdef _DEBUG
			return WindowImplBase::GetResourceType();
#else
			return UILIB_ZIPRESOURCE;
#endif
        }
        LPCTSTR CMainFrame::GetResourceID() const
        {
            // return WindowImplBase::GetResourceID();
            return MAKEINTRESOURCE(IDR_ZIP_RES);
        }
        CDuiString CMainFrame::GetSkinFolder()
        {
			#ifdef _DEBUG
				return _T("UIZIP");
			#else
				return CDuiString();
			#endif
        }
        CDuiString CMainFrame::GetSkinFile()
        {
            return _T("dlg_main_frame.xml");
        }
        LPCTSTR CMainFrame::GetWindowClassName(void) const
        {
            return _T("MyWeb::UI::CMainFrame");
        }
        void CMainFrame::OnCreateFrame()
        {
            ::PostThreadMessage(UIConfig().getUInt(Constants::key_nMainThreadId), WM_THREAD_MAIN_CREATE_FRAME, 0, 0);
        }

        void CMainFrame::InitListTable()
        {
            CListHeaderNumItem::ResetIndex();
            auto pHeader = m_pListTable->GetHeader();
            for (size_t i = 0; i < 5; ++i)
            {
                auto pHeaderNum = new CListHeaderNumItem;
				pHeaderNum->SetUserData(sm_UserNums[i].c_str());
                pHeader->Add(pHeaderNum);
                pHeader->Add(new CListHeaderDateItem);
            }

            m_pListTable->SetTextCallback(this);
        }

        void CMainFrame::InitList()
        {          
            TListInfoUI* pListInfo = m_pListMain->GetListInfo();
            {
                m_pVolume->SetBkColor(pListInfo->dwBkColor);
            }
        }

        void CMainFrame::OnHightlightDataByMouse(int nIndex, int nPos, bool isHighLight)
        {
            const auto& dataNums = m_data[nIndex].second;
            int nNum = dataNums[nPos].m_nNum;
            TGapItem gapItem = { nNum };
            {
                auto data = m_columns[nPos];
                auto adjacentFindRet = AdjacentFind(
                    std::begin(data), std::end(data), [](const TGapItem& lhs, const TGapItem& rhs) -> bool {
                    return lhs.m_nNum == rhs.m_nNum;
                }, gapItem);
                
            }
            {
                std::wstring strVal;
                for (auto it = std::cbegin(dataNums); it != std::cend(dataNums); ++it)
                {
                    strVal += std::to_wstring(it->m_nNum);
                    if ((it + 1) != std::cend(dataNums))
                    {
                        strVal += L",";
                    }
                }
                
                UpdateTitleText(m_data[nIndex].first, strVal);
            }
        }

        void CMainFrame::OnHightlightDataByCombo(int nPos, int nNum, bool isHighLight)
        {
            {
                auto data = m_columns[nPos];
                TGapItem gapItem = { nNum };
                auto adjacentFindRet = AdjacentFind(
                    std::begin(data), std::end(data), [](const TGapItem& lhs, const TGapItem& rhs) -> bool {
                    return lhs.m_nNum == rhs.m_nNum;
                }, gapItem);

                {
                    std::map<int, int> ret;
                    for (auto v : adjacentFindRet)
                    {
                        if (v.second != -1)
                        {
                            ret[v.second]++;
                        }
                    }
                    UpdateHorColumns(nPos, ret);
                }
            }
            {
                for (int i = 0; i < m_pListMain->GetCount(); ++i)
                {
                    auto pItemContainer = static_cast<CListContainerElementUI*>(m_pListMain->GetItemAt(i));
                    auto pSubContainer = static_cast<CMyListSubContainer*>(pItemContainer->GetItemAt(1 + nPos));
                    pSubContainer->HighLightItem(nNum);
                }
            }
        }

        void CMainFrame::UpdateTitleText(const std::wstring& strKey, const std::wstring& strVal)
        {
            CDuiString str;
            str.Format(L"%s期， (%s)", strKey.c_str(), strVal.c_str());
            m_pLblTitle->SetText(str);
        }

        void CMainFrame::UpdateStatusText(const std::wstring & strStatus)
        {
            m_pLblStatus->SetText(strStatus.c_str());
        }

        void CMainFrame::OnAddCaipiaoData(const TCaipiaoData& newData)
        {        
            m_allTheDatas[newData._date] = newData._data;

            // 为ListTable数据添加同步添加更多的显示元素
            for (size_t i = 0; i < newData._data.size(); i++)
            {
                CListTextElementUI* pListText = new CListTextElementUI;
                m_pListTable->Add(pListText);
            }
            CListLabelElementUI* pListElement = new CListLabelElementUI;
            pListElement->SetText(newData._date.c_str());
            m_pComboDate->Add(pListElement);

            if (m_pComboDate->GetCurSel() == -1)
            {
                m_pComboDate->SelectItem(0);
            }

            for (auto it = newData._data.begin(); it != newData._data.end(); ++it)
            {
                std::wstring str;
                for (auto v : it->second)
                {
                    str.push_back(v + '0');
                }
                for (size_t i = 0; i < it->second.size(); ++i)
                {
                    m_gapItemDatas[i].insert(std::make_pair(newData._date + L" " + it->first + L" " + str, it->second[i]));
                }
            }
        }

        void CMainFrame::OnUpdateCaipiaoData(const TCaipiaoData & newData)
        {
            m_allTheDatasUpdate[newData._date] = newData._data;
            CListLabelElementUI* pListElement = new CListLabelElementUI;
            pListElement->SetText(newData._date.c_str());
            m_pComboDate->Add(pListElement);

            if (m_pComboDate->GetCurSel() == -1)
            {
                m_pComboDate->SelectItem(0);
            }

            for (auto it = newData._data.begin(); it != newData._data.end(); ++it)
            {
                std::wstring str;
                for (auto v : it->second)
                {
                    str.push_back(v + '0');
                }
                for (size_t i = 0; i < it->second.size(); ++i)
                {
                    m_gapItemDatasUpdate[i].insert(std::make_pair(newData._date + L" " + it->first + L" " + str, it->second[i]));
                }
            }
        }

        void CMainFrame::RunDataUpdate()
        {
            UpdateStatusText(L"正在更新应用数据，请稍后......");
            m_gapItemDatasUpdate.clear();
            m_gapItemDatasUpdate.resize(5);
            m_allTheDatasUpdate.clear();
            m_pComboDate->RemoveAll();

            HWND hWnd = GetHWND();
            auto token = m_cts.get_token();
            const auto caipiaoDataPath = config().getString("application.dir") + "caipiao.data";
            auto task = concurrency::create_task([caipiaoDataPath, hWnd]() {
                std::string cmd("{\"Cmd\": \"UpdateCaiPiaoData\", \"datafilepath\": \"");
                cmd += caipiaoDataPath + "\"}";
                auto& uiSubSystem = Application::instance().getSubsystem<CUISubsystem>();
                {
                    std::wstring* pStrText = new std::wstring(L"正在爬取最新数据......");
                    ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                }
                return uiSubSystem.GetCaipiaoInfo(cmd);
            }, token);
            task.then([](std::string str) {
                Parser parser;

                Var result;
                try
                {
                    result = parser.parse(str);
                    auto obj = result.extract<Poco::JSON::Object::Ptr>();
                    const auto& resCode = obj->getValue<std::string>("rescode");
                    if (resCode != 1000)
                    {
                        ::MessageBox(NULL, NULL, NULL, MB_OK);
                        return false;
                    }
                    return true;
                }
                catch (JSONException& jsone)
                {
                    std::cout << jsone.message() << std::endl;
                    return false;
                }
                catch (...)
                {
                    return false;
                }
            }, token).then([caipiaoDataPath, hWnd, this](bool isSuccess) {
                Poco::File inputFile(caipiaoDataPath);
                if (inputFile.exists())
                {
                    Poco::FileInputStream fis(caipiaoDataPath);
                    std::string line;
                    std::vector<std::string> lines;
                    {
                        std::wstring* pStrText = new std::wstring(L"重新读取文件数据......");
                        ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                    }
                    logger().information(caipiaoDataPath);
                    while (std::getline(fis, line))
                    {
                        lines.push_back(line);
                    }
                    concurrency::parallel_for_each(lines.crbegin(), lines.crend(), [hWnd](const std::string& str)
                    {
                        Parser parser;
                        Var result;
                        try
                        {
                            result = parser.parse(str);
                            Object::Ptr object = result.extract<Object::Ptr>();

                            TCaipiaoData* pData = new TCaipiaoData;
                            const std::wstring strDate(object->getValue<std::wstring>("date"));
                            pData->_date = strDate;
                            auto spPhaseArr = object->getArray("phase");
                            for (auto spIt = spPhaseArr->begin(); spIt != spPhaseArr->end(); ++spIt)
                            {
                                auto spSubObj = spIt->extract<Object::Ptr>();
                                auto itFirst = spSubObj->begin();
                                auto& key = itFirst->first;
                                auto value = spSubObj->getValue<std::string>(key);
                                std::vector<int> nums(value.size());
                                for (size_t i = 0; i < value.size(); ++i)
                                {
                                    nums[i] = value[i] - '0';
                                }
                                pData->_data.emplace_back(A2W_UTF8(key), nums);
                            }

                            std::wstring* pStrText = new std::wstring(L"正在刷新");
                            pStrText->append(strDate);
                            pStrText->append(L"数据......");
                            ::PostMessage(hWnd, WM_USER_UPDATE_CAIPIAO_DATA, (WPARAM)pData, (LPARAM)pStrText);
                        }
                        catch (JSONException& jsone)
                        {
                            std::cout << jsone.message() << std::endl;
                        }
                        catch (Poco::SyntaxException& e)
                        {
                            std::cout << e.message() << std::endl;
                        }
                    });

                    std::wstring* pStrText = new std::wstring(L"已经成功刷新所有数据 @");
                    *pStrText += CDateTimeUtil::Now();
                    ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                    ::PostMessage(hWnd, WM_USER_LOAD_CAIPIAO_DATA_FINISH, 0, 0);
                }
            }, token);
        }

        void CMainFrame::RunDataInit()
        {
            UpdateStatusText(L"正在初始化数据，请稍后......");
            HWND hWnd = GetHWND();
            auto token = m_cts.get_token();
            const auto caipiaoDataPath = config().getString("application.dir") + "caipiao.data";
            auto task = concurrency::create_task([caipiaoDataPath, hWnd]() {
                std::string cmd("{\"Cmd\": \"UpdateCaiPiaoData\", \"datafilepath\": \"");
                cmd += caipiaoDataPath + "\"}";
                auto& uiSubSystem = Application::instance().getSubsystem<CUISubsystem>();
                {
                    std::wstring* pStrText = new std::wstring(L"正在爬取最新数据......");
                    ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                }
                return uiSubSystem.GetCaipiaoInfo(cmd);
            }, token);
            task.then([](std::string str) {
                Parser parser;

                Var result;
                try
                {
                    result = parser.parse(str);
                    auto obj = result.extract<Poco::JSON::Object::Ptr>();
                    const auto& resCode = obj->getValue<std::string>("rescode");
                    if (resCode != 1000)
                    {
                        ::MessageBox(NULL, NULL, NULL, MB_OK);
                        return false;
                    }
                    return true;
                }
                catch (JSONException& jsone)
                {
                    std::cout << jsone.message() << std::endl;
                    return false;
                }
                catch (...)
                {
                    return false;
                }
            }, token).then([caipiaoDataPath, hWnd, this](bool isSuccess) {
                Poco::File inputFile(caipiaoDataPath);
                if (inputFile.exists())
                {
                    Poco::FileInputStream fis(caipiaoDataPath);
                    std::string line;
                    std::vector<std::string> lines;
                    {
                        std::wstring* pStrText = new std::wstring(L"读取文件数据......");
                        ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                    }
                    logger().information(caipiaoDataPath);
                    while (std::getline(fis, line))
                    {
                        lines.push_back(line);
                    }
                    concurrency::parallel_for_each(lines.crbegin(), lines.crend(), [hWnd](const std::string& str)
                    {
                        Parser parser;
                        Var result;
                        try
                        {
                            result = parser.parse(str);
                            Object::Ptr object = result.extract<Object::Ptr>();

                            TCaipiaoData* pData = new TCaipiaoData;
                            const std::wstring strDate(object->getValue<std::wstring>("date"));
                            pData->_date = strDate;
                            auto spPhaseArr = object->getArray("phase");
                            for (auto spIt = spPhaseArr->begin(); spIt != spPhaseArr->end(); ++spIt)
                            {
                                auto spSubObj = spIt->extract<Object::Ptr>();
                                auto itFirst = spSubObj->begin();
                                auto& key = itFirst->first;
                                auto value = spSubObj->getValue<std::string>(key);
                                std::vector<int> nums(value.size());
                                for (size_t i = 0; i < value.size(); ++i)
                                {
                                    nums[i] = value[i] - '0';
                                }
                                pData->_data.emplace_back(A2W_UTF8(key), nums);
                            }

                            std::wstring* pStrText = new std::wstring(L"正在加载");
                            pStrText->append(strDate);
                            pStrText->append(L"数据......");
                            ::PostMessage(hWnd, WM_USER_ADD_CAIPIAO_DATA, (WPARAM)pData, (LPARAM)pStrText);
                        }
                        catch (JSONException& jsone)
                        {
                            std::cout << jsone.message() << std::endl;
                        }
                        catch (Poco::SyntaxException& e)
                        {
                            std::cout << e.message() << std::endl;
                        }
                    });

                    std::wstring* pStrText = new std::wstring(L"已经成功加载所有数据 @");
                    *pStrText += CDateTimeUtil::Now();
                    ::PostMessage(hWnd, WM_USER_UPDATE_STATUS_TEXT, (WPARAM)pStrText, 0);
                    ::PostMessage(hWnd, WM_USER_LOAD_CAIPIAO_DATA_FINISH, 0, 0);
                }
            }, token);
        }

        void CMainFrame::OnSelectComboDate(int nSelected)
        {
            m_selectedDate = m_pComboDate->GetItemAt(nSelected)->GetText();
            m_pListMain->RemoveAll();

            m_data.clear();

            const auto& newData = m_allTheDatas[m_selectedDate];

            for (auto it = newData.begin(); it != newData.end(); ++it)
            {
                auto& nums = it->second;
                m_data.push_back({ it->first,{ nums[0], nums[1], nums[2], nums[3], nums[4] } });
            }

            for (auto it = m_data.begin(); it != m_data.end(); ++it)
            {
                auto pContainer = new CMyListContainer;
                m_pListMain->Add(pContainer);
                pContainer->SetLeaderText(it->first);
                pContainer->UpdateFollowNums(it->second);
            }
        }

        void CMainFrame::OnSelectCombo0(int nSel)
        {
            OnHightlightDataByCombo(0, nSel, true);
        }

        void CMainFrame::OnSelectCombo1(int nSel)
        {
            OnHightlightDataByCombo(1, nSel, true);
        }

        void CMainFrame::OnSelectCombo2(int nSel)
        {
            OnHightlightDataByCombo(2, nSel, true);
        }

        void CMainFrame::OnSelectCombo3(int nSel)
        {
            OnHightlightDataByCombo(3, nSel, true);
        }

        void CMainFrame::OnSelectCombo4(int nSel)
        {
            OnHightlightDataByCombo(4, nSel, true);
        }

        void CMainFrame::UpdateHorColumns(int nIndex, const std::map<int, int>& data)
        {
            if (nIndex < 0 || nIndex > 4)
            {
                return;
            }
            CHorizontalLayoutUI* pHor = nullptr;
            switch (nIndex)
            {
            case 0:
                pHor = m_pHor0;
                break;
            case 1:
                pHor = m_pHor1;
                break;
            case 2:
                pHor = m_pHor2;
                break;
            case 3:
                pHor = m_pHor3;
                break;
            case 4:
                pHor = m_pHor4;
                break;
            default:
                break;
            }

            pHor->RemoveAll();
            for (auto v: data)
            {
                CVerticalLayoutUI* pVer = new CVerticalLayoutUI;
                pHor->Add(pVer);
                pVer->SetFixedWidth(27);
                CLabelUI* pLbl1 = new CLabelUI;
                pVer->Add(pLbl1);
                pLbl1->SetFont(0);
                pLbl1->SetText(std::to_wstring(v.first).c_str());
                CLabelUI* pLbl2 = new CLabelUI;
                pVer->Add(pLbl2);
                pLbl2->SetFont(0);
                pLbl2->SetText(std::to_wstring(v.second).c_str());
            }
        }

        LPCTSTR CMainFrame::GetItemText(CControlUI* pList, int iItem, int iSubItem)
        {
            int i = iSubItem / 2;

            auto& strTexts = m_tableTexts[i];

            if (iItem >= strTexts.size())
            {
                return L"";
            }

            if (iSubItem % 2 == 0)
            {
                return strTexts[iItem].first.c_str();
            }
            else
            {
                return strTexts[iItem].second.c_str();
            }
        }

        void CMainFrame::OnUpdateListTable(int nPos, const std::set<int>& nums)
        {
            auto data = m_gapItemDatas[nPos];

            auto adjacentFindRet = AdjacentFind<std::pair<std::wstring, int>>(
                std::begin(data), std::end(data), [](const std::pair<std::wstring, int>& lhs, 
                    const std::set<int>& nums) -> bool {
                return std::find(std::begin(nums), std::end(nums), lhs.second) != std::end(nums);
            }, nums);

            auto highLightHitNum = [nPos](int nNum, const std::wstring& strText) {
                std::wstring ret(strText);
                ret.insert(ret.size() - 5 + nPos, L"<c #ff0000>");
                ret.insert(ret.size() - 5 + nPos + 1, L"</c>");
                return ret;
            };

            auto& strText = m_tableTexts[nPos];
            strText.clear();
            for (auto it = adjacentFindRet.rbegin(); it != adjacentFindRet.rend(); ++it)
            {
                strText.emplace_back(std::make_pair(std::to_wstring(it->second), 
                    highLightHitNum(it->first.second, it->first.first)));
            }

            m_pListTable->Invalidate();
        }

        void CMainFrame::OnEditReturn(const std::wstring& strEdtName, const std::wstring& strText)
        {
            size_t nPos = std::stoi(strEdtName.substr(strEdtName.size() - 1, strEdtName.size()));
        
            std::set<int> nums;
            for (auto it = strText.cbegin(); it != strText.cend(); ++it)
            {
                nums.insert(*it - '0');
            }
            OnUpdateListTable(nPos, nums);
        }

    } // UI
} // MyWeb
