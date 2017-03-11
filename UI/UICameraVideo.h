#pragma once
#include <memory>
#include <map>
#include <thread>

namespace MyWeb {
    namespace Camera {
        class CFeatureExtraction;
        struct TAMVProperty;
        struct TCameraData;
    }

    namespace Voice {
        class CVoiceExtraction;
        struct TVoiceData;
    }
}

namespace DuiLib
{
    class CWebWnd;
    class CCameraVideoUI : public CContainerUI
    {
        struct CCameraVideoCtrl
        {
        };
        enum TWORK_THREAD_STATUS
        {
            THREAD_DEFAULT = 0x01,
            THREAD_CREATE = 0x02,
            THREAD_SUSPEND = 0x04,
        };
    public:
        static const CDuiString DUI_CTR_CAMERA_VIDEO;
    public:
        CCameraVideoUI();
        ~CCameraVideoUI();

        virtual LPCTSTR GetClass() const;
        virtual LPVOID GetInterface(LPCTSTR pstrName);

        virtual bool IsVisible() const;
        virtual void SetVisible(bool bVisible = true);

        void Start();
        void UpdateStartBtnStatus(int nThreadStatus);
        static void ResetIndex() {
            sm_nIndex = 0;
        }
    private:
        virtual void DoInit();

        void EnterWebThreadMessageLoop();
        bool OnThreadMessage(const MSG& msg, CPaintManagerUI* pManager, CCameraVideoCtrl* pCameraVideoCtrl, CWebWnd* pWebWnd);
        bool HandleWebMessage(CPaintManagerUI* pPaintManager, MSG &msg);
        void OnSetBrowserPos(LPRECT pRc, CCameraVideoCtrl* pWebCtrl, CWebWnd* pWebWnd);
        bool OnCreateCamera(CPaintManagerUI* pPaintManager, CCameraVideoCtrl* pWebCtrl, CWebWnd* pWebWnd);
        void OnQuitCamera();
        void OnShowBrowser(bool isVisible, CWebWnd* pWebWnd);
        bool OnCaptureVideo(CCameraVideoCtrl* pCameraVideoCtrl, bool isCapture);
        void OnAddCameraData(MyWeb::Camera::TCameraData* data);
        void OnAddVoiceData(MyWeb::Voice::TVoiceData* data);

        virtual void SetPos(RECT rc);
        void OnInvalideCameraWnd(HWND hWnd);
        
        void EmotionAnalysis();
    private:

        CWindowWnd* m_pBindWnd;

        DuiLib::CButtonUI* m_pBtnStart;

        static int sm_nIndex;

        int m_nThreadStatus;
        const int m_nIndex;
        HANDLE m_hInitEvent;
        volatile HWND m_hNotifyWnd;
        volatile DWORD m_dwWebWorkThreadId;
        DWORD m_dwMainThreadId;
        std::vector<HANDLE> m_dwSubmodulesThreadHandles;
        std::vector<DWORD> m_dwSubmodulesThreadIds;
        Poco::BasicEvent<void> OnQuit;

        HANDLE m_hThread;
        size_t m_cameraFps;

        std::string m_strWndName;

        std::vector<std::shared_ptr<MyWeb::Camera::TCameraData>> m_cameraDatas;
        std::vector<std::shared_ptr<MyWeb::Voice::TVoiceData>> m_voiceDatas;
        std::map<long, std::shared_ptr<MyWeb::Camera::TAMVProperty>> m_amvProperties;
    };
} // namespace DuiLib
