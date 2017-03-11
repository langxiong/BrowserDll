#pragma once

#include <WinUser.h>
#include <tchar.h>
#include <string>
#include <array>

namespace MyWeb {

    namespace UI {

        struct TEmotionAnalysisResult
        {
        };

        enum UI_MESSAGES
        {
            MY_USER_MSG_BEGIN = WM_USER + 0x008,
            WM_THREAD_MAIN_CREATE_FRAME, // 主线程创建Frame窗口
            WM_THREAD_MAIN_CREATE_RENDER_PARENT_WND, // 主线程创建Render的父窗口
            WM_THREAD_FRAME_QUIT, // Frame窗口退出


            WM_USER_CREATE_FRAME,
            WM_USER_ADD_WORKTHREAD_HANDLE,
			WM_USER_UPDATE_DATA,
            WM_USER_ADD_EMOTION_ANALYSIS_RESULT,

            WM_THREAD_CAMERA_VIDEO_BEGIN,
            WM_USER_CAMERA_VIDEO_SIZE_CHANGED,
            WM_USER_SHOW_CAMERA_VIDEO,
            WM_USER_CV_MAINFRAME_CREATED,
            WM_USER_CV_BUTTON_CLICK,
            WM_THREAD_START_CAMERA,
            WM_THREAD_STOP_CAMERA,
            WM_THREAD_ADD_WORKTHREAD_HANDLE,
            WM_THREAD_ADD_EMOTION_HISTORY_DATA,
            WM_THREAD_ADD_VOICE_HISTORY_DATA,
            WM_THREAD_UPDATE_CAMERA_FPS,
            WM_THREAD_QUIT_CAMERA_VIDEO,
            WM_THREAD_INVALIDE_CAMERA_WND,
            WM_THREAD_CAPTURE_CAMERA_VIDEO,
            WM_THREAD_TIMER_CAPTURE_CAMERA_VIDEO,
            WM_THREAD_CAMERA_VIDEO_END,
        };

        enum
        {
            FLAG_UPDATE_CAMERA_DATA = 0x01,
            FLAG_UPDATE_VOICE_DATA
        };

        namespace Constants
        {
            static const std::string key_nMainThreadId = "key_nMainThreadId";
            static const std::string key_nTimerUpdateUserData = "key_nTimerUpdateUserData";
        } // Constants

    } // UI
} // MyWeb
