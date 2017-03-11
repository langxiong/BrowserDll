#include "stdafx.h"
#include "FrameMgr.h"
#include <thread>
#include "MainFrame.h"

namespace MyWeb {
    namespace UI {
        uint32_t CFrameMgr::sm_frameID = 0;
        CFrameMgr::CFrameMgr(DWORD dwMainThreadID):
            m_dwMainThreadID(dwMainThreadID)
        {
        }

        CFrameMgr::~CFrameMgr()
        {
        }

        void CFrameMgr::OnCreateFrame(const void* pSender)
        {
            uint32_t frameID = sm_frameID++;
            m_mainFrames.insert(std::make_pair(frameID, std::thread([this, frameID]()
            {
                CMainFrame* pMainFrame = new CMainFrame(frameID);
                pMainFrame->CreateDuiWindow(NULL, _T("MyWeb::UI::MainFrame"), UI_WNDSTYLE_FRAME);
                int ret = pMainFrame->ShowModal();
                delete pMainFrame;
                ::PostThreadMessage(m_dwMainThreadID, WM_THREAD_FRAME_QUIT, frameID, 0);
            })));
            m_mainFrames[frameID].detach();
        }
        void CFrameMgr::OnQuitFrame(const void * pSender, const uint32_t& frameID)
        {
            m_mainFrames.erase(frameID);
            if (m_mainFrames.empty())
            {
                ::PostThreadMessage(m_dwMainThreadID, WM_QUIT, 0, 0);
            }
        }
    } // UI
} // MyWeb
