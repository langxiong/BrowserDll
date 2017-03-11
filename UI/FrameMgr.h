#pragma once

namespace MyWeb {
     namespace UI {
         class CFrameMgr
         {
         public:
             explicit CFrameMgr(DWORD dwMainThreadID);
             ~CFrameMgr();

             void OnCreateFrame(const void* pSender);
             void OnQuitFrame(const void* pSender, const uint32_t& frameID);
         private:

             static uint32_t sm_frameID;

             DWORD m_dwMainThreadID;
             std::map<DWORD, std::thread> m_mainFrames;
         };
    } // UI
} // MyWeb
