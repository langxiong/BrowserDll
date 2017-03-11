#include "StdAfx.h"
#include <algorithm>

#include "AppCommon.h"
#include "StringHelper.h"

#include "Poco/Delegate.h"
#include "Poco/DateTimeFormatter.h"
#include "DateTimeUtil.h"

namespace DuiLib
{
    class CHostWnd : public CWindowWnd
    {
    public:
        virtual LPCTSTR GetWindowClassName() const
        {
            return _T("HostWndForCameraVideoByXL");
        }

        virtual void OnFinalMessage(HWND hWnd)
        {
            CWindowWnd::OnFinalMessage(hWnd);
        }
    };

    class CCameraContainer : public CControlUI
    {
    public:
        CCameraContainer()
        {

        }

        virtual void DoEvent(TEventUI& event)
        {
            if (event.Type == UIEVENT_MOUSEENTER)
            {
                m_pManager->SendNotify(this, DUI_MSGTYPE_MENU, event.wParam, event.lParam);
                return;
            }

			return CControlUI::DoEvent(event);
        }

    };
    
    class CWebWnd :
        public CWindowWnd,
        public INotifyUI

    {
    public:
        CWebWnd(CPaintManagerUI* pPaintManager, DWORD dwWebWorkThreadId):
            m_pPaintManager(pPaintManager),
            m_dwWorkThreadId(dwWebWorkThreadId)
        {

        }

        ~CWebWnd()
        {
            ::PostQuitMessage(0);
        }

        void WaitWndClosed()
        {
        }

        virtual LPCTSTR GetWindowClassName() const
        {
            return _T("CameraVideoWndByXL");
        }

        virtual void OnFinalMessage(HWND hWnd)
        {
            CWindowWnd::OnFinalMessage(hWnd);
        }

        virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
        {
            if (WM_USER_CV_MAINFRAME_CREATED == uMsg)
            {
                HWND hCreated = (HWND)wParam;
				HWND hCamera = (HWND)lParam;
                RECT rc = { 0 };
                ::GetClientRect(GetHWND(), &rc);
				::SetWindowPos(hCreated, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER);
				::SetWindowPos(hCamera, NULL, 0, 0, rc.right - rc.left, rc.bottom - rc.top, SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER);
                return S_OK;
            }

            if (WM_USER_CV_BUTTON_CLICK == uMsg)
            {
                ::PostThreadMessage(m_dwWorkThreadId, WM_THREAD_STOP_CAMERA, 0, 0);
                return S_OK;
            }

            LRESULT lRes = S_OK;
            if (m_pPaintManager->MessageHandler(uMsg, wParam, lParam, lRes))
                return lRes;

            return CWindowWnd::HandleMessage(uMsg, wParam, lParam);
        }

        virtual void Notify(TNotifyUI& msg)
        {
        }

    private:
        CPaintManagerUI* m_pPaintManager;
        DWORD m_dwWorkThreadId;
    };

    const CDuiString CCameraVideoUI::DUI_CTR_CAMERA_VIDEO(_T("CameraVideo"));
    int CCameraVideoUI::sm_nIndex;

    CCameraVideoUI::CCameraVideoUI():
        m_pBindWnd(NULL),
        m_pBtnStart(NULL),
        m_nThreadStatus(THREAD_DEFAULT),
        m_nIndex(sm_nIndex++),
        m_hInitEvent(NULL),
        m_hNotifyWnd(NULL),
        m_dwWebWorkThreadId(0),
        m_dwMainThreadId(0),
        m_hThread(NULL),
        m_cameraFps(20)
    {
        SetBkColor(0xff1a0e0d);
        m_pBtnStart = new CButtonUI;
        Add(m_pBtnStart);
        m_pBtnStart->SetName(_T("start#btn"));
        m_pBtnStart->SetFixedWidth(141);
        m_pBtnStart->SetFixedHeight(141);
        m_pBtnStart->SetBkImage(_T("start.png"));
    }

    CCameraVideoUI::~CCameraVideoUI()
    {
        if (m_pBindWnd)
        {
            delete m_pBindWnd;
        }
    }

    void CCameraVideoUI::DoInit()
    {
        if (m_pBindWnd)
        {
            return;
        }
        m_pBindWnd = new CHostWnd;
        m_dwMainThreadId = ::GetCurrentThreadId();
        if (m_pManager)
        {
            m_hNotifyWnd = m_pManager->GetPaintWindow();

            DWORD dwStyle = WS_CHILD | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
            if (IsVisible())
            {
                dwStyle |= WS_VISIBLE;
            }
            HWND hWnd = m_pBindWnd->CreateDuiWindow(m_pManager->GetPaintWindow(), NULL, dwStyle);
            if (!hWnd)
            {
                return;
            }
            m_hInitEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
            std::thread t([&]()
            {
                EnterWebThreadMessageLoop();
            });
            t.detach();
            ::WaitForSingleObject(m_hInitEvent, INFINITE);
            m_hThread = ::OpenThread(SYNCHRONIZE, FALSE, m_dwWebWorkThreadId);
            ::SendMessage(m_hNotifyWnd, WM_USER_ADD_WORKTHREAD_HANDLE, (WPARAM)m_hThread, m_dwWebWorkThreadId);
        }
    }

    LPCTSTR CCameraVideoUI::GetClass() const
    {
        return _T("CameraVideoUI");
    }

    LPVOID CCameraVideoUI::GetInterface(LPCTSTR pstrName)
    {
        if (_tcscmp(pstrName, DUI_CTR_CAMERA_VIDEO) == 0) return static_cast<CCameraVideoUI*>(this);
        return CContainerUI::GetInterface(pstrName);
    }

    void CCameraVideoUI::SetPos(RECT rc)
    {
        CControlUI::SetPos(rc);

        if ((m_nThreadStatus & THREAD_CREATE) != THREAD_CREATE)
        {
            RECT rcStartBtn(rc);
            rcStartBtn.left = (rcStartBtn.left + rcStartBtn.right) / 2  - m_pBtnStart->GetFixedWidth() / 2;
            rcStartBtn.right = rcStartBtn.left + m_pBtnStart->GetFixedWidth();

            rcStartBtn.top = (rcStartBtn.top + rcStartBtn.bottom) / 2 - m_pBtnStart->GetFixedHeight() /2 ;
            rcStartBtn.bottom = rcStartBtn.top + m_pBtnStart->GetFixedHeight();

            m_pBtnStart->SetPos(rcStartBtn);
			return;
        }

		if ((m_nThreadStatus & THREAD_SUSPEND) != THREAD_SUSPEND)
		{
			if (m_pBindWnd && IsVisible())
			{
				::SetWindowPos(m_pBindWnd->GetHWND(), NULL, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top,
					SWP_NOACTIVATE | SWP_NOREDRAW | SWP_NOZORDER | SWP_SHOWWINDOW);
				LPRECT pRC = new RECT;
				pRC->left = pRC->top = 0;
				pRC->right = rc.right - rc.left;
				pRC->bottom = rc.bottom - rc.top;

				if (::PostThreadMessage(m_dwWebWorkThreadId, WM_USER_CAMERA_VIDEO_SIZE_CHANGED, (WPARAM)pRC, NULL))
				{

				}
				else
				{
					delete pRC;
				}
			}
		}
		else
		{
			RECT rcStartBtn(rc);
			rcStartBtn.left = (rcStartBtn.left + rcStartBtn.right) / 2 - m_pBtnStart->GetFixedWidth() / 2;
			rcStartBtn.right = rcStartBtn.left + m_pBtnStart->GetFixedWidth();

			rcStartBtn.top = (rcStartBtn.top + rcStartBtn.bottom) / 2 - m_pBtnStart->GetFixedHeight() / 2;
			rcStartBtn.bottom = rcStartBtn.top + m_pBtnStart->GetFixedHeight();

			m_pBtnStart->SetPos(rcStartBtn);
		}
       
    }

    bool CCameraVideoUI::IsVisible() const
    {
        return CContainerUI::IsVisible();
        bool isVisible = CContainerUI::IsVisible();
        if (isVisible && m_pBindWnd)
        {
            return ::IsWindowVisible(m_pBindWnd->GetHWND()) ? true : false;
        }
        return isVisible;
    }

    void CCameraVideoUI::SetVisible(bool bVisible /*= true*/)
    {
        CContainerUI::SetVisible(bVisible);
        
        if (m_pBindWnd)
        {
            m_pBindWnd->ShowWindow(bVisible);
        }
        ::PostThreadMessage(m_dwWebWorkThreadId, WM_USER_SHOW_CAMERA_VIDEO, bVisible ? TRUE : FALSE, NULL);
    }

    void CCameraVideoUI::EnterWebThreadMessageLoop()
    {
        CPaintManagerUI* pManager = new CPaintManagerUI;
        CCameraVideoCtrl* pCameraVideoCtrl = new CCameraVideoCtrl;
        m_dwWebWorkThreadId = ::GetCurrentThreadId();
        CWebWnd* pWebWnd = new CWebWnd(pManager, m_dwWebWorkThreadId);
        MSG msg = { 0 };
        HRESULT hr = ::CoInitialize(nullptr);
        ::PeekMessage(&msg, NULL, NULL, NULL, PM_NOREMOVE);

        ::SetEvent(m_hInitEvent);

        while (::GetMessage(&msg, nullptr, 0, 0) != 0)
        {
            if (OnThreadMessage(msg, pManager, pCameraVideoCtrl, pWebWnd))
            {
                continue;
            }
               
            if (!HandleWebMessage(pManager, msg))
            {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
        }

        if (m_nThreadStatus & THREAD_SUSPEND)
        {
            CFeatureExtraction::Quit();
        }

        OnQuit.notify(nullptr);
        if (m_hInitEvent)
        {
            ::CloseHandle(m_hInitEvent);
        }
        
        if (pCameraVideoCtrl)
        {
            delete pCameraVideoCtrl;
            pCameraVideoCtrl = nullptr;
        }
        if (::IsWindow(pWebWnd->GetHWND()))
        {
            pWebWnd->Close();
        }
        if (pManager)
        {
            delete pManager;
        }
        if (pWebWnd)
        {
            delete pWebWnd;
        }
        ::CoUninitialize();
    }

    bool CCameraVideoUI::OnThreadMessage(const MSG& msg, CPaintManagerUI* pManager, CCameraVideoCtrl* pCameraVideoCtrl, CWebWnd* pWebWnd)
    {
        if (msg.message == WM_THREAD_START_CAMERA)
        {
            if ((m_nThreadStatus & THREAD_CREATE) != THREAD_CREATE)
            {
                if (!OnCreateCamera(pManager, pCameraVideoCtrl, pWebWnd))
                {
                    return false;
                }
                m_nThreadStatus |= THREAD_CREATE;
            }
			else
			{
                ::SetWindowPos(m_pBindWnd->GetHWND(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_SHOWWINDOW);
                CFeatureExtraction::Resume();
				m_nThreadStatus &= ~THREAD_SUSPEND;
			}
            ::PostMessage(m_hNotifyWnd, WM_THREAD_INVALIDE_CAMERA_WND, NULL, NULL);
            return true;
        }

        if (msg.message == WM_THREAD_STOP_CAMERA)
        {
            if ((m_nThreadStatus & THREAD_SUSPEND) == THREAD_SUSPEND)
            {
                // already suspend thread. return imeditately.
                return true;
            }

            CFeatureExtraction::Pause();
            ::SetWindowPos(m_pBindWnd->GetHWND(), NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED | SWP_HIDEWINDOW);

            m_nThreadStatus |= THREAD_SUSPEND;
            ::PostMessage(m_hNotifyWnd, WM_THREAD_STOP_CAMERA, m_nThreadStatus, NULL);
            return true;
        }

        if (msg.message == WM_THREAD_ADD_WORKTHREAD_HANDLE)
        {
            if (msg.wParam)
            {
                m_dwSubmodulesThreadHandles.push_back(reinterpret_cast<HANDLE>(msg.wParam));
            }
            if (msg.lParam)
            {
                m_dwSubmodulesThreadIds.push_back(msg.lParam);
            }
            return true;
        }

        if (msg.message == WM_THREAD_ADD_EMOTION_HISTORY_DATA)
        {
            MyWeb::Camera::TCameraData* pData = reinterpret_cast<MyWeb::Camera::TCameraData*>(msg.lParam);
            if (pData)
            {
                OnAddCameraData(pData);
            }
            return true;
        }

        if (msg.message == WM_THREAD_CAPTURE_CAMERA_VIDEO)
        {
            OnCaptureVideo(pCameraVideoCtrl, msg.wParam ? true : false);
            return true;
        }

        if (msg.message == WM_USER_CAMERA_VIDEO_SIZE_CHANGED)
        {
            LPRECT pRc = reinterpret_cast<LPRECT>(msg.wParam);
            if (!pRc)
            {
                return true;
            }
            if (pManager->GetRoot())
            {
                pManager->GetRoot()->SetPos({ 0, 0, pRc->right - pRc->left, pRc->bottom - pRc->top });
            }
            OnSetBrowserPos(pRc, pCameraVideoCtrl, pWebWnd);
            delete pRc;
            return true;
        }

        if (msg.message == WM_USER_SHOW_CAMERA_VIDEO)
        {
            OnShowBrowser(msg.wParam ? true : false, pWebWnd);
            return true;
        }

        if (msg.message == WM_THREAD_ADD_VOICE_HISTORY_DATA)
        {
            MyWeb::Voice::TVoiceData* pData = reinterpret_cast<MyWeb::Voice::TVoiceData*>(msg.lParam);
            if (pData)
            {
                OnAddVoiceData(pData);
            }
            return true;
        }

        if (msg.message == WM_THREAD_UPDATE_CAMERA_FPS)
        {
            if (msg.wParam)
            {
                m_cameraFps = static_cast<size_t>(msg.wParam);
            }
            return true;
        }

        return false;
    }

    bool CCameraVideoUI::OnCreateCamera(CPaintManagerUI* pPaintManager, CCameraVideoCtrl* pCameraVideoCtrl, CWebWnd* pWebWnd)
    {
        m_pBindWnd->ShowWindow(true);
        DWORD dwStyle = IsVisible() ? (WS_VISIBLE | WS_POPUP) : WS_POPUP;
        HWND hWnd = pWebWnd->CreateDuiWindow(NULL, NULL, dwStyle);

        pPaintManager->Init(hWnd, false);
        CCameraContainer* pC = new CCameraContainer;
        pC->SetBkColor(0x0);
        pPaintManager->AttachDialog(pC);
        pPaintManager->AddNotifier(pWebWnd);

        LONG styleValue = ::GetWindowLong(pWebWnd->GetHWND(), GWL_STYLE);
        styleValue &= ~WS_POPUP;
        ::SetWindowLong(pWebWnd->GetHWND(), GWL_STYLE, styleValue | WS_CHILD);
        ::SetParent(pWebWnd->GetHWND(), m_pBindWnd->GetHWND());
        
        CCameraSubsystem::SetOwner(pWebWnd->GetHWND());

#if 1

        std::thread t1([this]()
        {
            auto dwThreadId = ::GetCurrentThreadId();
            auto hHandle = ::OpenThread(SYNCHRONIZE | THREAD_SUSPEND_RESUME, FALSE, dwThreadId);
            ::SendMessage(m_hNotifyWnd, WM_USER_ADD_WORKTHREAD_HANDLE, (WPARAM)hHandle, dwThreadId);
            ::PostThreadMessage(m_dwWebWorkThreadId, WM_THREAD_ADD_WORKTHREAD_HANDLE, (WPARAM)hHandle, dwThreadId);

            struct TCameraEvent
            {
                TCameraEvent(MyWeb::Camera::CFeatureExtraction& featureExtraction):
                    m_featureExtraction(featureExtraction),
                    m_hNotifyWnd(nullptr),
                    m_dwWorkThreadId(0)
                {

                }

                void SetNotifyWnd(HWND hNotifyWnd)
                {
                    m_hNotifyWnd = hNotifyWnd;
                }

                void SetNotifyThreadId(DWORD dwNotifyTHreadId)
                {
                    m_dwWorkThreadId = dwNotifyTHreadId;
                }

                void OnQuit(const void * pSender)
                {
                    m_featureExtraction.Quit();
                }

                void OnFpsUpdate(const void * pSender, const size_t& nCameraFps)
                {
                    DWORD ret = ::PostThreadMessage(m_dwWorkThreadId, WM_THREAD_UPDATE_CAMERA_FPS, nCameraFps, NULL);
                }

                void OnDataUpdate(const void * pSender, const MyWeb::Camera::TCameraData& data)
                {
                    {
                        MyWeb::Camera::TCameraData* pData(new MyWeb::Camera::TCameraData(data));
                        DWORD ret = ::PostMessage(m_hNotifyWnd, WM_USER_UPDATE_DATA, FLAG_UPDATE_CAMERA_DATA, (LPARAM)(pData));
                        if (ret == 0)
                        {
                            delete pData;
                        }
                    }
                    {
                        MyWeb::Camera::TCameraData* pData(new MyWeb::Camera::TCameraData(data));
                        DWORD ret = ::PostThreadMessage(m_dwWorkThreadId, WM_THREAD_ADD_EMOTION_HISTORY_DATA, NULL, (LPARAM)(pData));
                        if (ret == 0)
                        {
                            delete pData;
                        }
                    }
                }

                MyWeb::Camera::CFeatureExtraction& m_featureExtraction;
                HWND m_hNotifyWnd;
                DWORD m_dwWorkThreadId;
            };

            MyWeb::Camera::CFeatureExtraction featureExtraction;
            featureExtraction.SetOwner(m_hNotifyWnd);

            TCameraEvent e(featureExtraction);
            e.SetNotifyWnd(m_hNotifyWnd);
            e.SetNotifyThreadId(m_dwWebWorkThreadId);
            MyWeb::Camera::CFeatureExtraction::OnFpsUpdate += Poco::delegate(&e, &TCameraEvent::OnFpsUpdate);
            MyWeb::Camera::CFeatureExtraction::OnDataUpdate += Poco::delegate(&e, &TCameraEvent::OnDataUpdate);
            OnQuit += Poco::delegate(&e, &TCameraEvent::OnQuit);
            featureExtraction.Start();
            featureExtraction.Run();
            OnQuit -= Poco::delegate(&e, &TCameraEvent::OnQuit);
        });

        t1.detach();
#endif

#if 1
        std::thread t2([&pCameraVideoCtrl, this]()
        {
            auto dwThreadId = ::GetCurrentThreadId();
            auto hHandle = ::OpenThread(SYNCHRONIZE | THREAD_SUSPEND_RESUME, FALSE, dwThreadId);
            ::SendMessage(m_hNotifyWnd, WM_USER_ADD_WORKTHREAD_HANDLE, (WPARAM)hHandle, dwThreadId);
            ::PostThreadMessage(m_dwWebWorkThreadId, WM_THREAD_ADD_WORKTHREAD_HANDLE, (WPARAM)hHandle, dwThreadId);

            struct TVoiceEvent
            {
                TVoiceEvent(MyWeb::Voice::CVoiceExtraction& voiceExtraction) :
                    m_voiceExtraction(voiceExtraction),
                    m_hNotifyWnd(nullptr),
                    m_dwWorkThreadId(0)
                {

                }

                void SetNotifyWnd(HWND hNotifyWnd)
                {
                    m_hNotifyWnd = hNotifyWnd;
                }

                void SetNotifyThreadId(DWORD dwNotifyTHreadId)
                {
                    m_dwWorkThreadId = dwNotifyTHreadId;
                }

                void OnQuit(const void * pSender)
                {
                    m_voiceExtraction.quit();
                }

                void OnDataUpdate(const void * pSender, MyWeb::Voice::TVoiceData& data)
                {
                    {
                        MyWeb::Voice::TVoiceData* pData(new MyWeb::Voice::TVoiceData(data));
                        DWORD ret = ::PostMessage(m_hNotifyWnd, WM_USER_UPDATE_DATA, FLAG_UPDATE_VOICE_DATA, (LPARAM)(pData));
                        if (ret == 0)
                        {
                            delete pData;
                        }
                    }
                    {
                        MyWeb::Voice::TVoiceData* pData(new MyWeb::Voice::TVoiceData(data));
                        DWORD ret = ::PostThreadMessage(m_dwWorkThreadId, WM_THREAD_ADD_VOICE_HISTORY_DATA, NULL, (LPARAM)(pData));
                        if (ret == 0)
                        {
                            delete pData;
                        }
                    }
                }
                MyWeb::Voice::CVoiceExtraction& m_voiceExtraction;
                HWND m_hNotifyWnd;
                DWORD m_dwWorkThreadId;
            };
            MyWeb::Voice::CVoiceExtraction voiceExtraction;
            TVoiceEvent e(voiceExtraction);
            e.SetNotifyWnd(m_hNotifyWnd);
            e.SetNotifyThreadId(m_dwWebWorkThreadId);
            MyWeb::Voice::CVoiceExtraction::OnData += Poco::delegate(&e, &TVoiceEvent::OnDataUpdate);
            OnQuit += Poco::delegate(&e, &TVoiceEvent::OnQuit);
            voiceExtraction.start();
            OnQuit -= Poco::delegate(&e, &TVoiceEvent::OnQuit);
        });
        t2.detach();
#endif
        return true;
    }

    void CCameraVideoUI::OnQuitCamera()
    {
    }

    void CCameraVideoUI::OnSetBrowserPos(LPRECT pRc, CCameraVideoCtrl* pCameraVideoCtrl, CWebWnd* pWebWnd)
    {
        if ((m_nThreadStatus & THREAD_SUSPEND) == THREAD_SUSPEND)
        {
            return;
        }
        ::SetWindowPos(pWebWnd->GetHWND(), NULL, 0, 0, pRc->right - pRc->left, pRc->bottom - pRc->top,
            SWP_NOACTIVATE | SWP_NOREDRAW );

        HWND hTrackbar = CCameraSubsystem::GetTrackbarHwnd();
        if (hTrackbar)
        {
            ::SetWindowPos(hTrackbar, NULL, 0, 0, pRc->right - pRc->left, pRc->bottom - pRc->top,
                SWP_NOACTIVATE | SWP_NOREDRAW );
        }

		HWND hCamera = CCameraSubsystem::GetCameraHwnd();
		if (hCamera)
		{
			::SetWindowPos(hCamera, NULL, 0, 0, pRc->right - pRc->left, pRc->bottom - pRc->top,
				SWP_NOACTIVATE | SWP_NOREDRAW );
		}
    }

    void CCameraVideoUI::OnShowBrowser(bool isVisible, CWebWnd* pWebWnd)
    {
        if (pWebWnd)
        {
            pWebWnd->ShowWindow(isVisible);
        }
    }

    void CCameraVideoUI::OnAddCameraData(MyWeb::Camera::TCameraData* pData)
    {
        if (m_cameraDatas.size() > 1000)
        {
            m_cameraDatas = std::vector<std::shared_ptr<MyWeb::Camera::TCameraData>>(
                m_cameraDatas.cbegin() + 800, m_cameraDatas.cend());
        }
        m_cameraDatas.push_back(std::shared_ptr<MyWeb::Camera::TCameraData>(pData));
    }

    void CCameraVideoUI::OnAddVoiceData(MyWeb::Voice::TVoiceData* pData)
    {
        if (m_voiceDatas.size() > 1000)
        {
            m_voiceDatas = std::vector<std::shared_ptr<MyWeb::Voice::TVoiceData>>(
                m_voiceDatas.cbegin() + 800, m_voiceDatas.cend());
        }
        m_voiceDatas.push_back(std::shared_ptr<MyWeb::Voice::TVoiceData>(pData));
        // 触发情绪分析
        EmotionAnalysis();
    }

    bool CCameraVideoUI::HandleWebMessage(CPaintManagerUI* pPaintManager, MSG &msg)
    {
        bool isWebPaintManagerHandle = false;
        if (msg.hwnd == pPaintManager->GetPaintWindow())
        {
            isWebPaintManagerHandle = true;
        }
        else
        {
            HWND hWndParent = NULL;
            UINT uStyle = GetWindowStyle(hWndParent);
            if (uStyle & WS_CHILD)
            {
                hWndParent = ::GetParent(msg.hwnd);
            }
            while (hWndParent)
            {
                if (hWndParent == pPaintManager->GetPaintWindow())
                {
                    isWebPaintManagerHandle = true;
                    break;
                }

                uStyle = GetWindowStyle(hWndParent);
                if (uStyle & WS_CHILD)
                {
                    hWndParent = ::GetParent(hWndParent);
                }
                else
                {
                    break;
                }
            }
        }

        if (isWebPaintManagerHandle)
        {
            if (pPaintManager->TranslateAccelerator(&msg))
            {
                return true;
            }
            LRESULT lRes = 0;
            if (pPaintManager->PreMessageHandler(msg.message, msg.wParam, msg.lParam, lRes))
            {
                return true;
            }
        }
        return false;
    }

    bool CCameraVideoUI::OnCaptureVideo(CCameraVideoCtrl* pCameraVideoCtrl, bool isCapture)
    {
        return true;
    }

    void CCameraVideoUI::OnInvalideCameraWnd(HWND hWnd)
    {
        RECT rc = { 0 };
        ::GetClientRect(hWnd, &rc);
        ::InvalidateRect(hWnd, &rc, FALSE);
    }

    void CCameraVideoUI::EmotionAnalysis()
    {
        auto spLatestData = m_voiceDatas.back();

		if (spLatestData->_keywords.empty())
		{
			return;
		}

        std::vector<size_t> motions(7);
        for (auto v : m_cameraDatas)
        {
            if (v->_timestamp >= spLatestData->_startTime &&
                v->_timestamp <= spLatestData->_endTime)
            {
                auto maxIt = std::max_element(std::begin(v->_radarDatas), std::end(v->_radarDatas));
                auto n = std::distance(std::begin(v->_radarDatas), maxIt);

                if (*maxIt > EMOTION_THRESHOLD[n])
                {
                    motions[n]++;
                }
            }
        }

        auto timeDiff = spLatestData->_endTime - spLatestData->_startTime;

        //auto frameCount = timeDiff / 1000 * m_cameraFps;
        auto frameCount = 0.5*m_cameraFps;
        
        std::cout << "FrameCount:　" << frameCount << std::endl;
        std::vector<std::wstring> ret;
        for (size_t i = 0; i < 7; ++i)
        {
            if (motions[i] > 0 && motions[i] > (FRAME_RATE[i] * frameCount))
            {
                ret.emplace_back(EMOTION_WORD[i]);
            }
        }

        if (!ret.empty())
        {
            std::wstring timeStr(MyWeb::CDateTimeUtil::FormatLocalDateTime(spLatestData->_startTime, "%H:%M:%S"));

            std::wstring* pStr = new std::wstring(timeStr + L" 客户在谈到`");
            auto& str = *pStr;
            size_t i = 0;
			for (; i < spLatestData->_keywords.size() - 1; i++)
			{
				str += A2W_GB2312(spLatestData->_keywords[i]);
				str += L",";
			}
            str += A2W_GB2312(spLatestData->_keywords[i]);
            str += L"`表现出";
            for (size_t i = 0; i < (ret.size() - 1); ++i)
            {
                str += L"<c #FF9912>";
                str += ret[i];
                str += L"、";
            }
            if (ret.size() == 1)
			{
				str += L"<c #FF9912>";
			}
            str += ret.back();
            str += L"</c>";
            str += L"的表情";

			str += L"</f>";
            ::PostMessage(m_hNotifyWnd, WM_USER_ADD_EMOTION_ANALYSIS_RESULT, (WPARAM)pStr, NULL);
        }

    }

    void CCameraVideoUI::Start()
    {
        if (m_dwWebWorkThreadId)
        {
            if (::PostThreadMessage(m_dwWebWorkThreadId, WM_THREAD_START_CAMERA, 0, 0))
            {
                m_pBtnStart->SetVisible(false);
            }
            
        }
    }

    void CCameraVideoUI::UpdateStartBtnStatus(int nThreadStatus)
    {
        if ((nThreadStatus & THREAD_SUSPEND) == THREAD_SUSPEND)
        {
            m_pBtnStart->SetBkImage(_T("stop.png"));
            m_pBtnStart->SetVisible(true);
            return;
        }
		else
		{
			m_pBtnStart->SetVisible(false);
		}
    }

} // DuiLib
