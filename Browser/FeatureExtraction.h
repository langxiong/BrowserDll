
#pragma once

#include <windows.h>
#include "MyCamera.h"
#include "../Voice/VoiceDefine.h"
#include "Poco/Mutex.h"

namespace MyWeb {
     namespace Camera {
		 class MYCAMERA_API CFeatureExtraction
		 {
         public:
             enum TCaptureStatus
             {
                STATUS_RUNNING = 0x01,
                STATUS_PAUSE = 0x02,
                STATUS_NEED_BASE_LINE = 0x04,
                STATUS_QUIT = 0x08,
             };

             enum TEvents
             {
                 EVT_CAMERA_RUNNING = 0,
                 EVT_CAMERA_QUIT,
             };
		 public:
             CFeatureExtraction();
             ~CFeatureExtraction();

			 void SetOwner(HWND hOwner);
			 HWND GetOwner() const;

             void Initialize();
             void Uninitialize();

			 void Run();

             const std::vector<std::vector<double>>& GetDatas() const;

             static void Start();
             static void Pause();
             static void Resume();
             static void Quit();

             static Poco::BasicEvent<const TCameraData> OnDataUpdate;
             static Poco::BasicEvent<const size_t> OnFpsUpdate;

         public:

             // todo: remove static
             static std::vector<std::vector<double>> m_datas;
             static std::vector<double> init_AU_average;

             static int sm_nStatus;
             static Poco::Mutex sm_statusMutex;


             static std::vector<HANDLE> sm_hEvents;
         private:

             HWND m_hOwner;

             std::vector<std::string> m_args;

		 };
    } // Camera
} // MyWeb