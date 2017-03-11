#include "StdAfx.h"
#include "FeatureExtraction.h"
#include "CameraSubsystem.h"
#include "AppCommon.h"
//#include "../Voice/MyVoice.h"
// OpenCV includes
#include <opencv2/videoio/videoio.hpp>  // Video write
#include <opencv2/videoio/videoio_c.h>  // Video write
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/algorithm/string.hpp>
#include "Poco/Delegate.h"

// Local includes
#include "LandmarkDetector/LandmarkCoreIncludes.h"
#include <FaceAnalyser/Face_utils.h>
#include <FaceAnalyser/FaceAnalyser.h>
#include <FaceAnalyser/GazeEstimation.h>

#pragma warning(disable:4995)

using namespace Poco;
using namespace boost::filesystem;
namespace MyWeb {
    namespace Camera {

        #define AU_MAX 17
        #define QUEUE_LENGTH_MAX 280
        #define MAX_EMOTION_SECOND 30
        #define EMOTION_CNT 7

        #ifndef CONFIG_DIR
        #define CONFIG_DIR "~"
        #endif

        #define INFO_STREAM( stream ) \
                     std::cout << stream << std::endl

        #define WARN_STREAM( stream ) \
                     std::cout << "Warning: " << stream << std::endl

        #define ERROR_STREAM( stream ) \
                     std::cout << "Error: " << stream << std::endl

        #define FATAL_STREAM( stream ) \
                    printErrorAndAbort( std::string( "Fatal error: " ) + stream )

        // Some globals for tracking timing information for visualisation
        double fps_tracker = -1.0;
        int64 t0 = 0;
        double fps_vid_in = -1.0;
        std::vector<double> EMOTION_THRESHOLD = { 0.5, 0.5, 0.5, 0.5, 0.5, 0.5, 0.5 };
        std::vector<std::string> EMOTION_WORD = { "∆¯∑ﬂ", "√Ô ”","—·∂Ò","ø÷æÂ","”‰‘√","±Ø…À","æ™—»" };

        //std::vector<std::vector<double>> emotion_prediction_hist;
        // std::vector<ActionUnitForTransmisson> ausForTransmission;
        // queue<std::vector<double>> ausForTransmission;

        void prepareOutputFile(std::ofstream* output_file, bool output_2D_landmarks, bool output_3D_landmarks,
            bool output_model_params, bool output_pose, bool output_AUs, bool output_gaze,
            int num_landmarks, int num_model_modes, std::vector<std::string> au_names_class, std::vector<std::string> au_names_reg);

        void output_HOG_frame(std::ofstream* hog_file, bool good_frame, const cv::Mat_<double>& hog_descriptor, int num_rows, int num_cols);

        void lowpassFilter(std::vector<double>& auForTransmission);

        void visualise_tracking(cv::Mat& captured_image, const LandmarkDetector::CLNF& face_model, const LandmarkDetector::FaceModelParameters& det_parameters, cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, int frame_count, double fx, double fy, double cx, double cy);

        void post_process_output_file(FaceAnalysis::FaceAnalyser& face_analyser, std::string output_file, bool dynamic);

        void get_image_input_output_params_feats(std::vector<std::vector<std::string> > &input_image_files, bool& as_video, std::vector<std::string> &arguments);

        void get_output_feature_params(std::vector<std::string> &output_similarity_aligned, std::vector<std::string> &output_hog_aligned_files, double &similarity_scale,
            int &similarity_size, bool &grayscale, bool& verbose, bool& dynamic,
            bool &output_2D_landmarks, bool &output_3D_landmarks, bool &output_model_params, bool &output_pose, bool &output_AUs, bool &output_gaze,
            std::vector<std::string> &arguments);

        void printErrorAndAbort(const std::string & error);

        std::vector<std::vector<double>> CFeatureExtraction::m_datas(17);
        std::vector<double> CFeatureExtraction::init_AU_average(17);

        int CFeatureExtraction::sm_nStatus;

        Poco::Mutex CFeatureExtraction::sm_statusMutex;

        std::vector<HANDLE> CFeatureExtraction::sm_hEvents;

        CFeatureExtraction::CFeatureExtraction() :
			m_hOwner(nullptr)
        {
            for (auto& v : m_datas)
            {
                v.resize(QUEUE_LENGTH_MAX);
            }
            Initialize();
        }

        CFeatureExtraction::~CFeatureExtraction()
        {
            Uninitialize();
        }

		void CFeatureExtraction::SetOwner(HWND hOwner)
		{
			m_hOwner = hOwner;
		}

		HWND CFeatureExtraction::GetOwner() const
		{
			return m_hOwner;
		}

        void CFeatureExtraction::Initialize()
        {
            for (size_t i = EVT_CAMERA_RUNNING; i <= EVT_CAMERA_QUIT; ++i)
            {
                HANDLE hEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
                sm_hEvents.emplace_back(hEvent);
            }
        }

        void CFeatureExtraction::Uninitialize()
        {
            for (auto hEvent: sm_hEvents)
            {
                ::CloseHandle(hEvent);
            }
        }

        void init_capture_baseline(FaceAnalysis::FaceAnalyser& face_analyser, cv::VideoCapture& video_capture, LandmarkDetector::CLNF& face_model, LandmarkDetector::FaceModelParameters& det_parameters, cv::Mat& captured_image, bool use_world_coordinates, double fps_vid_in, double time_stamp, double total_frames, int& frame_count, int& reported_completion, int fx, int fy, int cx, int cy) {
            std::vector<double> init_AU_average(17);
            int init_emotion_prediction_cnt = fps_vid_in * 5;
            video_capture >> captured_image;
            for (int init_emotion_prediction_point = 0; !captured_image.empty() && init_emotion_prediction_point < init_emotion_prediction_cnt; init_emotion_prediction_point++) {
                // Grab the timestamp first
                {
                    Poco::ScopedLock<Poco::Mutex> lock(CFeatureExtraction::sm_statusMutex);
                    if ((CFeatureExtraction::sm_nStatus & CFeatureExtraction::STATUS_PAUSE)
                        == CFeatureExtraction::STATUS_PAUSE)
                    {
                        return;
                    }
                    if ((CFeatureExtraction::sm_nStatus & CFeatureExtraction::STATUS_QUIT)
                        == CFeatureExtraction::STATUS_QUIT)
                    {
                        return;
                    }

                }
               
                time_stamp = (double)frame_count * (1.0 / fps_vid_in);

                // Reading the images
                cv::Mat_<uchar> grayscale_image;

                if (captured_image.channels() == 3)
                {
                    cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
                }
                else
                {
                    grayscale_image = captured_image.clone();
                }

                // The actual facial landmark detection / tracking
                bool detection_success;

                detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, face_model, det_parameters);

                // Gaze tracking, absolute gaze direction
                cv::Point3f gazeDirection0(0, 0, -1);
                cv::Point3f gazeDirection1(0, 0, -1);

                if (det_parameters.track_gaze && detection_success && face_model.eye_model)
                {
                    FaceAnalysis::EstimateGaze(face_model, gazeDirection0, fx, fy, cx, cy, true);
                    FaceAnalysis::EstimateGaze(face_model, gazeDirection1, fx, fy, cx, cy, false);
                }

                // Do face alignment
                cv::Mat sim_warped_img;
                cv::Mat_<double> hog_descriptor;

                // Work out the pose of the head from the tracked model
                cv::Vec6d pose_estimate;

                face_analyser.AddNextFrame(captured_image, face_model, time_stamp, false, !det_parameters.quiet_mode);
                face_analyser.GetLatestAlignedFace(sim_warped_img);

                if (!det_parameters.quiet_mode)
                {
                    cv::imshow("sim_warp", sim_warped_img);
                }

                if (use_world_coordinates)
                {
                    pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(face_model, fx, fy, cx, cy);
                }
                else
                {
                    pose_estimate = LandmarkDetector::GetCorrectedPoseCamera(face_model, fx, fy, cx, cy);
                }
                visualise_tracking(captured_image, face_model, det_parameters, gazeDirection0, gazeDirection1, frame_count, fx, fy, cx, cy);
                double visualisation_boundary = 0.2;

                // output AU
                auto aus_reg = face_analyser.GetCurrentAUsReg();
                std::vector<double> auForTransmission;
                std::vector<std::string> au_reg_names = face_analyser.GetAURegNames();
                std::vector<double> emotion_predictions = face_analyser.GetEmotionPredictions();

                if (aus_reg.size() == 0 || face_model.detection_certainty > visualisation_boundary) {
                    for (size_t p = 0; p < face_analyser.GetAURegNames().size(); ++p) {
                        std::cout << ", 0";
                        auForTransmission.push_back(0);
                    }
                    for (size_t p = 0; p < emotion_predictions.size(); ++p) {
                        emotion_predictions[p] = 0;
                    }
                }
                else {
                    // write out ar the correct index
                    for (std::string au_name : au_reg_names) {
                        for (auto au_reg : aus_reg) {
                            if (au_name.compare(au_reg.first) == 0) {
                                //if (au_reg.second < 0) au_reg.second = 0;
                                auForTransmission.push_back(au_reg.second);
                                std::cout << ", " << au_reg.second;
                                break;
                            }
                        }
                    }
                }

                for (size_t i = 0; i < auForTransmission.size(); ++i) {
                    init_AU_average[i] += auForTransmission[i];
                }

                video_capture >> captured_image;

                // detect key presses
                char character_press = cv::waitKey(1);

                // restart the tracker
                if (character_press == 'r')
                {
                    face_model.Reset();
                }

                // Update the frame count
                frame_count++;

                if (total_frames != -1)
                {
                    if ((double)frame_count / (double)total_frames >= reported_completion / 10.0)
                    {
                        reported_completion = reported_completion + 1;
                    }
                }
            }
            for (size_t i = 0; i < init_AU_average.size(); ++i)
            {
                init_AU_average[i] = init_AU_average[i] / init_emotion_prediction_cnt;
                std::cout << init_AU_average[i] << endl;
            }
            face_analyser.setAUBaseLine(init_AU_average);
        }

        void CFeatureExtraction::Run()
        {
            m_args.emplace_back(config().getString("application.dir"));
            m_args.emplace_back("-mloc");
            m_args.emplace_back("model/main_clnf_wild.txt");
            m_args.emplace_back("-wild");

            /*MyWeb::Voice::CVoiceExtraction voice_extraction;
            voice_extraction.start();*/

            boost::filesystem::path config_path = boost::filesystem::path(CONFIG_DIR);
            boost::filesystem::path parent_path = boost::filesystem::path(m_args[0]).parent_path();
            // Some initial parameters that can be overriden from command line	
            std::vector<std::string> input_files, depth_directories, output_files, tracked_videos_output;

            LandmarkDetector::FaceModelParameters det_parameters(m_args);
            // Always track gaze in feature extraction
            det_parameters.track_gaze = true;

            // Get the input output file parameters

            // Indicates that rotation should be with respect to camera or world coordinates
            bool use_world_coordinates;
            std::string output_codec; //not used but should
            LandmarkDetector::get_video_input_output_params(input_files, depth_directories, output_files, tracked_videos_output, use_world_coordinates, output_codec, m_args);

            bool video_input = true;
            bool verbose = true;
            bool images_as_video = false;

            std::vector<std::vector<std::string> > input_image_files;

            // Adding image support for reading in the files
            if (input_files.empty())
            {
                std::vector<std::string> d_files;
                std::vector<std::string> o_img;
                std::vector<cv::Rect_<double>> bboxes;
                get_image_input_output_params_feats(input_image_files, images_as_video, m_args);

                if (!input_image_files.empty())
                {
                    video_input = false;
                }

            }

            // Grab camera parameters, if they are not defined (approximate values will be used)
            float fx = 640, fy = 684, cx = 640, cy = 684;
            int d = 0;
            // Get camera parameters
            LandmarkDetector::get_camera_params(d, fx, fy, cx, cy, m_args);

            // If cx (optical axis centre) is undefined will use the image size/2 as an estimate
            bool cx_undefined = false;
            bool fx_undefined = false;
            if (cx == 0 || cy == 0)
            {
                cx_undefined = true;
            }
            if (fx == 0 || fy == 0)
            {
                fx_undefined = true;
            }

            // The modules that are being used for tracking
            LandmarkDetector::CLNF face_model(det_parameters.model_location);

            std::vector<std::string> output_similarity_align;
            std::vector<std::string> output_hog_align_files;

            double sim_scale = 0.7;
            int sim_size = 112;
            bool grayscale = false;
            bool video_output = false;
            bool dynamic = true; // Indicates if a dynamic AU model should be used (dynamic is useful if the video is long enough to include neutral expressions)
            int num_hog_rows;
            int num_hog_cols;

            // By default output all parameters, but these can be turned off to get smaller files or slightly faster processing times
            // use -no2Dfp, -no3Dfp, -noMparams, -noPose, -noAUs, -noGaze to turn them off
            bool output_2D_landmarks = true;
            bool output_3D_landmarks = true;
            bool output_model_params = true;
            bool output_pose = true;
            bool output_AUs = true;
            bool output_gaze = true;

            get_output_feature_params(output_similarity_align, output_hog_align_files, sim_scale, sim_size, grayscale, verbose, dynamic,
                output_2D_landmarks, output_3D_landmarks, output_model_params, output_pose, output_AUs, output_gaze, m_args);

            // Used for image masking

            std::string tri_loc;
            if (boost::filesystem::exists(path("model/tris_68_full.txt")))
            {
                tri_loc = "model/tris_68_full.txt";
            }
            else
            {
                path loc = path(m_args[0]).parent_path() / "model/tris_68_full.txt";
                tri_loc = loc.string();

                if (!exists(loc))
                {
                    std::cout << "Can't find triangulation files, exiting" << std::endl;
                    return;
                }
            }

            // Will warp to scaled mean shape
            cv::Mat_<double> similarity_normalised_shape = face_model.pdm.mean_shape * sim_scale;
            // Discard the z component
            similarity_normalised_shape = similarity_normalised_shape(cv::Rect(0, 0, 1, 2 * similarity_normalised_shape.rows / 3)).clone();

            // If multiple video files are tracked, use this to indicate if we are done
            bool done = false;
            int f_n = -1;
            int curr_img = -1;

            std::string au_loc;

            std::string au_loc_local;

            if (dynamic)
            {
                au_loc_local = "AU_predictors/AU_all_best.txt";
            }
            else
            {
                au_loc_local = "AU_predictors/AU_all_static.txt";
            }

            if (boost::filesystem::exists(path(au_loc_local)))
            {
                au_loc = au_loc_local;
            }
            else
            {
                path loc = path(m_args[0]).parent_path() / au_loc_local;

                if (exists(loc))
                {
                    au_loc = loc.string();
                }
                else
                {
                    std::cout << "Can't find AU prediction files, exiting" << std::endl;
                    return;
                }
            }

            /****************************************************************************************************************/
            /************************************find emotion classifier start***********************************************/
            /****************************************************************************************************************/

            std::string em_loc;
            std::string em_loc_local;
            em_loc_local = "emotion_classifiers/emotion_classifier_3.dat";

            boost::filesystem::path em_loc_path = boost::filesystem::path(em_loc_local);
            if (boost::filesystem::exists(em_loc_path))
            {
                em_loc = em_loc_path.string();
            }
            else if (boost::filesystem::exists(parent_path / em_loc_path))
            {
                em_loc = (parent_path / em_loc_path).string();
            }
            else if (boost::filesystem::exists(config_path / em_loc_path))
            {
                em_loc = (config_path / em_loc_path).string();
            }
            else
            {
                cout << "Can't find emotion classifier files, exiting" << endl;
                return;
            }
            /****************************************************************************************************************/
            /*************************************find emotion classifier end************************************************/
            /****************************************************************************************************************/

            // Creating a  face analyser that will be used for AU extraction
            FaceAnalysis::FaceAnalyser face_analyser(std::vector<cv::Vec3d>(), 0.7, 112, 112, au_loc, tri_loc, em_loc);

            while (!done) // this is not a for loop as we might also be reading from a webcam
            {
                std::string current_file;

                cv::VideoCapture video_capture;

                cv::Mat captured_image;
                int total_frames = -1;
                int reported_completion = 0;


                if (video_input)
                {
                    // We might specify multiple video files as arguments
                    if (input_files.size() > 0)
                    {
                        f_n++;
                        current_file = input_files[f_n];
                    }
                    else
                    {
                        // If we want to write out from webcam
                        f_n = 0;
                    }
                    // Do some grabbing
                    if (current_file.size() > 0)
                    {
                        INFO_STREAM("Attempting to read from file: " << current_file);
                        video_capture = cv::VideoCapture(current_file);
                        total_frames = (int)video_capture.get(CV_CAP_PROP_FRAME_COUNT);
                        fps_vid_in = video_capture.get(CV_CAP_PROP_FPS);

                        // Check if fps is nan or less than 0
                        if (fps_vid_in != fps_vid_in || fps_vid_in <= 0)
                        {
                            INFO_STREAM("FPS of the video file cannot be determined, assuming 30");
                            fps_vid_in = 30;
                        }
                    }
                    /********************* cz modification test begin **********************
                    /*********************
                    /*********************
                    /*********************
                    /*********************/
                    else
                    {
                        INFO_STREAM("Attempting to capture from device: " << d);
                        video_capture = cv::VideoCapture(d);

                        video_capture.set(cv::CAP_PROP_FRAME_WIDTH, 800);
                        video_capture.set(cv::CAP_PROP_FRAME_HEIGHT, 600);

                        auto nWidth = video_capture.get(cv::CAP_PROP_FRAME_WIDTH);
                        auto nHeight = video_capture.get(cv::CAP_PROP_FRAME_HEIGHT);
                        // Read a first frame often empty in camera
                        cv::Mat captured_image;
                        video_capture >> captured_image;

                        total_frames = (int)video_capture.get(CV_CAP_PROP_FRAME_COUNT);
                        fps_vid_in = video_capture.get(CV_CAP_PROP_FPS);
                        CFeatureExtraction::OnFpsUpdate.notify(nullptr, fps_vid_in);
                        // check if fps is nan or less than 0
                        if (fps_vid_in != fps_vid_in || fps_vid_in <= 0) {
                            INFO_STREAM("FPS of the device cannot be determined, assuming 30");
                            fps_vid_in = 30;
                        }

                    }
                    /*********************
                    /*********************
                    /*********************
                    /*********************/
                    /********************* cz modification test end *******************/

                    if (!video_capture.isOpened())
                    {
                        FATAL_STREAM("Failed to open video source, exiting");
                        return;
                    }
                    else
                    {
                        INFO_STREAM("Device or file opened");
                    }

                    video_capture >> captured_image;
                }
                else
                {
                    f_n++;
                    curr_img++;
                    if (!input_image_files[f_n].empty())
                    {
                        std::string curr_img_file = input_image_files[f_n][curr_img];
                        captured_image = cv::imread(curr_img_file, -1);
                    }
                    else
                    {
                        FATAL_STREAM("No .jpg or .png images in a specified drectory, exiting");
                        return;
                    }

                }

                // If optical centers are not defined just use center of image
                if (cx_undefined)
                {
                    cx = captured_image.cols / 2.0f;
                    cy = captured_image.rows / 2.0f;
                }
                // Use a rough guess-timate of focal length
                if (fx_undefined)
                {
                    fx = 500 * (captured_image.cols / 640.0);
                    fy = 500 * (captured_image.rows / 480.0);

                    fx = (fx + fy) / 2.0;
                    fy = fx;
                }

                // Creating output files
                std::ofstream output_file;
                // Saving the HOG features
                std::ofstream hog_output_file;
                if (!output_hog_align_files.empty())
                {
                    hog_output_file.open(output_hog_align_files[f_n], ios_base::out | ios_base::binary);
                }

                // saving the videos
                cv::VideoWriter writerFace;
                if (!tracked_videos_output.empty())
                {
                    try
                    {
                        writerFace = cv::VideoWriter(tracked_videos_output[f_n], CV_FOURCC(output_codec[0], output_codec[1], output_codec[2], output_codec[3]), fps_vid_in, captured_image.size(), true);
                    }
                    catch (cv::Exception e)
                    {
                        WARN_STREAM("Could not open VideoWriter, OUTPUT FILE WILL NOT BE WRITTEN. Currently using codec " << output_codec << ", try using an other one (-oc option)");
                    }
                }

                int frame_count = 0;

                // This is useful for a second pass run (if want AU predictions)
                std::vector<cv::Vec6d> params_global_video;
                std::vector<bool> successes_video;
                std::vector<cv::Mat_<double>> params_local_video;
                std::vector<cv::Mat_<double>> detected_landmarks_video;

                // Use for timestamping if using a webcam
                int64 t_initial = cv::getTickCount();

                bool visualise_hog = verbose;

                // Timestamp in seconds of current processing
                double time_stamp = 0;

                INFO_STREAM("Starting tracking");

                {
                    // wait start event.
                    int tmpStatus = 0;
                    {
                        Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);
                        tmpStatus = sm_nStatus;
                    }
                    if ((tmpStatus & STATUS_RUNNING) != STATUS_RUNNING)
                    {
                        DWORD ret = ::WaitForMultipleObjectsEx(sm_hEvents.size(), &sm_hEvents[0], TRUE, INFINITE, FALSE);
                        switch (ret)
                        {
                        case EVT_CAMERA_QUIT:
                        {
                            return;
                        }
                        default:
                            break;
                        }
                    }
                }

                while (!captured_image.empty())
                {
                    {
                        int tmpStatus = 0;
                        {
                            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);
                            tmpStatus = sm_nStatus;
                        }
                        if ((tmpStatus & STATUS_QUIT) == STATUS_QUIT)
                        {
                            return;
                        }

                        if ((tmpStatus & STATUS_PAUSE) == STATUS_PAUSE)
                        {
                            MSG msg = { 0 };
                            while (::GetMessage(&msg, nullptr, 0, 0) != 0)
                            {
                                ::TranslateMessage(&msg);
                                ::DispatchMessage(&msg);

                                Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);
                                if ((sm_nStatus & STATUS_PAUSE) != STATUS_PAUSE)
                                {
                                    tmpStatus = sm_nStatus;
                                    break;
                                }

                                if ((sm_nStatus & STATUS_QUIT) == STATUS_QUIT)
                                {
                                    return;
                                }
                            }
                        }

                        // the first 10 second to record initial history
                        if ((tmpStatus & STATUS_NEED_BASE_LINE) == STATUS_NEED_BASE_LINE)
                        {
                            init_capture_baseline(face_analyser, video_capture, face_model, det_parameters, captured_image, use_world_coordinates, fps_vid_in, time_stamp, total_frames, frame_count, reported_completion, fx, fy, cx, cy);
                            tmpStatus &= ~STATUS_NEED_BASE_LINE;
                            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);
                            sm_nStatus &= ~STATUS_NEED_BASE_LINE;
                        }
                    }

                    // Grab the timestamp first
                    if (video_input)
                    {
                        time_stamp = (double)frame_count * (1.0 / fps_vid_in);
                    }
                    else
                    {
                        // if loading images assume 30fps
                        time_stamp = (double)frame_count * (1.0 / 30.0);
                    }

                    // Reading the images
                    cv::Mat_<uchar> grayscale_image;

                    if (captured_image.channels() == 3)
                    {
                        cvtColor(captured_image, grayscale_image, CV_BGR2GRAY);
                    }
                    else
                    {
                        grayscale_image = captured_image.clone();
                    }

                    // The actual facial landmark detection / tracking
                    bool detection_success;

                    if (video_input || images_as_video)
                    {
                        detection_success = LandmarkDetector::DetectLandmarksInVideo(grayscale_image, face_model, det_parameters);
                    }
                    else
                    {
                        detection_success = LandmarkDetector::DetectLandmarksInImage(grayscale_image, face_model, det_parameters);
                    }

                    // Gaze tracking, absolute gaze direction
                    cv::Point3f gazeDirection0(0, 0, -1);
                    cv::Point3f gazeDirection1(0, 0, -1);

                    if (det_parameters.track_gaze && detection_success && face_model.eye_model)
                    {
                        FaceAnalysis::EstimateGaze(face_model, gazeDirection0, fx, fy, cx, cy, true);
                        FaceAnalysis::EstimateGaze(face_model, gazeDirection1, fx, fy, cx, cy, false);
                    }

                    // Do face alignment
                    cv::Mat sim_warped_img;
                    cv::Mat_<double> hog_descriptor;

                    face_analyser.AddNextFrame(captured_image, face_model, time_stamp, false, !det_parameters.quiet_mode);
                    face_analyser.GetLatestAlignedFace(sim_warped_img);

                    // Work out the pose of the head from the tracked model
                    cv::Vec6d pose_estimate;
                    if (use_world_coordinates)
                    {
                        pose_estimate = LandmarkDetector::GetCorrectedPoseWorld(face_model, fx, fy, cx, cy);
                    }
                    else
                    {
                        pose_estimate = LandmarkDetector::GetCorrectedPoseCamera(face_model, fx, fy, cx, cy);
                    }

                    /*if (hog_output_file.is_open())
                    {
                        output_HOG_frame(&hog_output_file, detection_success, hog_descriptor, num_hog_rows, num_hog_cols);
                    }*/

                    // Write the similarity normalised output
                    if (!output_similarity_align.empty())
                    {

                        if (sim_warped_img.channels() == 3 && grayscale)
                        {
                            cvtColor(sim_warped_img, sim_warped_img, CV_BGR2GRAY);
                        }

                        char name[100];

                        // output the frame number
                        std::sprintf(name, "frame_det_%06d.bmp", frame_count);

                        // Construct the output filename
                        boost::filesystem::path slash("/");

                        std::string preferredSlash = slash.make_preferred().string();

                        std::string out_file = output_similarity_align[f_n] + preferredSlash + std::string(name);
                        bool write_success = imwrite(out_file, sim_warped_img);

                        if (!write_success)
                        {
                            std::cout << "Could not output similarity aligned image image" << std::endl;
                            return;
                        }
                    }

                    // Visualising the tracker
                    visualise_tracking(captured_image, face_model, det_parameters, gazeDirection0, gazeDirection1, frame_count, fx, fy, cx, cy);

                    /********************* cz modification test begin **********************/
                    double visualisation_boundary = 0.2;
                    if (output_AUs) {
                        auto aus_reg = face_analyser.GetCurrentAUsReg();
                        // ActionUnitForTransmisson auForTransmission;
                        std::vector<double> auForTransmission;

                        std::vector<std::string> au_reg_names = face_analyser.GetAURegNames();
                        std::sort(au_reg_names.begin(), au_reg_names.end());

                        std::vector<double> emotion_predictions = face_analyser.GetEmotionPredictions();

                        if (aus_reg.size() == 0 || face_model.detection_certainty > visualisation_boundary) {
                            for (size_t p = 0; p < face_analyser.GetAURegNames().size(); ++p) {
                                std::cout << ", 0";
                                auForTransmission.push_back(0);
                            }
                            for (size_t p = 0; p < emotion_predictions.size(); ++p) {
                                emotion_predictions[p] = 0;
                            }
                        }
                        else {
                            // write out ar the correct index
                            for (std::string au_name : au_reg_names) {
                                for (auto au_reg : aus_reg) {
                                    if (au_name.compare(au_reg.first) == 0) {
                                        if (au_reg.second < 0) au_reg.second = 0;
                                        auForTransmission.push_back(au_reg.second);
                                        //std::cout << ", " << au_reg.second;
                                        break;
                                    }
                                }
                            }
                        }

                        assert(auForTransmission.size() == m_datas.size());
                        for (size_t i = 0; i < auForTransmission.size(); ++i)
                        {
                            auto& v = m_datas[i];
                            v.erase(v.begin());
                            v.push_back(auForTransmission[i]);
                        }
                        lowpassFilter(auForTransmission);

                        TCameraData tmpDatas = {
                            auForTransmission,
                            emotion_predictions,
                            Poco::Timestamp()
                        };

                        CFeatureExtraction::OnDataUpdate.notify(nullptr, tmpDatas);
                        //std::vector<double> emotion_pred = face_analyser.GetEmotionPredictions();
                    }

                    /*********************
                    /*********************
                    /*********************
                    /*********************/
                    /********************* cz modification test end *******************/

                    // output the tracked video
                    if (!tracked_videos_output.empty())
                    {
                        writerFace << captured_image;
                    }

                    if (video_input)
                    {
                        video_capture >> captured_image;
                    }
                    else
                    {
                        curr_img++;
                        if (curr_img < (int)input_image_files[f_n].size())
                        {
                            std::string curr_img_file = input_image_files[f_n][curr_img];
                            captured_image = cv::imread(curr_img_file, -1);
                        }
                        else
                        {
                            captured_image = cv::Mat();
                        }
                    }
                    // detect key presses
                    char character_press = cv::waitKey(1);

                    // restart the tracker
                    if (character_press == 'r')
                    {
                        face_model.Reset();
                    }
                    else if (character_press == 'i')
                    {
                        break;
                    }

                    // Update the frame count
                    frame_count++;

                    if (total_frames != -1)
                    {
                        if ((double)frame_count / (double)total_frames >= reported_completion / 10.0)
                        {
                            std::cout << reported_completion * 10 << "% ";
                            reported_completion = reported_completion + 1;
                        }
                    }

                }

                output_file.close();

                /*if (output_files.size() > 0 && output_AUs)
                {
                    std::cout << "Postprocessing the Action Unit predictions" << std::endl;
                    post_process_output_file(face_analyser, output_files[f_n], dynamic);
                }*/
                // Reset the models for the next video
                face_analyser.Reset();
                face_model.Reset();

                frame_count = 0;
                curr_img = -1;

                if (total_frames != -1)
                {
                    std::cout << std::endl;
                }

                // break out of the loop if done with all the files (or using a webcam)
                if ((video_input && f_n == input_files.size() - 1) || (!video_input && f_n == input_image_files.size() - 1))
                {
                    done = true;
                }
            }
        }
        void printErrorAndAbort(const std::string & error)
        {
            std::cout << error << std::endl;
        }
        
        std::vector<std::string> get_arguments(int argc, char **argv)
        {
            std::vector<std::string> arguments;

            // First argument is reserved for the name of the executable
            for (int i = 0; i < argc; ++i)
            {
                arguments.push_back(std::string(argv[i]));
            }
            return arguments;
        }
        
        // Useful utility for creating directories for storing the output files
        void create_directory_from_file(std::string output_path)
        {

            // Creating the right directory structure

            // First get rid of the file
            auto p = boost::filesystem::path(boost::filesystem::path(output_path).parent_path());

            if (!p.empty() && !boost::filesystem::exists(p))
            {
                bool success = boost::filesystem::create_directories(p);
                if (!success)
                {
                    std::cout << "Failed to create a directory... " << p.string() << std::endl;
                }
            }
        }
        
        void create_directory(std::string output_path)
        {

            // Creating the right directory structure
            auto p = boost::filesystem::path(output_path);

            if (!boost::filesystem::exists(p))
            {
                bool success = boost::filesystem::create_directories(p);

                if (!success)
                {
                    std::cout << "Failed to create a directory..." << p.string() << std::endl;
                }
            }
        }

        void get_output_feature_params(std::vector<std::string> &output_similarity_aligned, std::vector<std::string> &output_hog_aligned_files, double &similarity_scale,
            int &similarity_size, bool &grayscale, bool& verbose, bool& dynamic,
            bool &output_2D_landmarks, bool &output_3D_landmarks, bool &output_model_params, bool &output_pose, bool &output_AUs, bool &output_gaze,
            std::vector<std::string> &arguments)
        {
            output_similarity_aligned.clear();
            output_hog_aligned_files.clear();

            bool* valid = new bool[arguments.size()];

            for (size_t i = 0; i < arguments.size(); ++i)
            {
                valid[i] = true;
            }

            std::string output_root = "";

            // By default the model is dynamic
            dynamic = true;

            std::string separator = std::string(1, boost::filesystem::path::preferred_separator);

            // First check if there is a root argument (so that videos and outputs could be defined more easilly)
            for (size_t i = 0; i < arguments.size(); ++i)
            {
                if (arguments[i].compare("-root") == 0)
                {
                    output_root = arguments[i + 1] + separator;
                    i++;
                }
                if (arguments[i].compare("-outroot") == 0)
                {
                    output_root = arguments[i + 1] + separator;
                    i++;
                }
            }

            for (size_t i = 0; i < arguments.size(); ++i)
            {
                if (arguments[i].compare("-simalign") == 0)
                {
                    output_similarity_aligned.push_back(output_root + arguments[i + 1]);
                    create_directory(output_root + arguments[i + 1]);
                    valid[i] = false;
                    valid[i + 1] = false;
                    i++;
                }
                else if (arguments[i].compare("-hogalign") == 0)
                {
                    output_hog_aligned_files.push_back(output_root + arguments[i + 1]);
                    create_directory_from_file(output_root + arguments[i + 1]);
                    valid[i] = false;
                    valid[i + 1] = false;
                    i++;
                }
                else if (arguments[i].compare("-verbose") == 0)
                {
                    verbose = true;
                }
                else if (arguments[i].compare("-au_static") == 0)
                {
                    dynamic = false;
                }
                else if (arguments[i].compare("-g") == 0)
                {
                    grayscale = true;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-simscale") == 0)
                {
                    similarity_scale = stod(arguments[i + 1]);
                    valid[i] = false;
                    valid[i + 1] = false;
                    i++;
                }
                else if (arguments[i].compare("-simsize") == 0)
                {
                    similarity_size = stoi(arguments[i + 1]);
                    valid[i] = false;
                    valid[i + 1] = false;
                    i++;
                }
                else if (arguments[i].compare("-no2Dfp") == 0)
                {
                    output_2D_landmarks = false;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-no3Dfp") == 0)
                {
                    output_3D_landmarks = false;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-noMparams") == 0)
                {
                    output_model_params = false;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-noPose") == 0)
                {
                    output_pose = false;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-noAUs") == 0)
                {
                    output_AUs = false;
                    valid[i] = false;
                }
                else if (arguments[i].compare("-noGaze") == 0)
                {
                    output_gaze = false;
                    valid[i] = false;
                }
            }

            for (int i = arguments.size() - 1; i >= 0; --i)
            {
                if (!valid[i])
                {
                    arguments.erase(arguments.begin() + i);
                }
            }

        }
        
        // Can process images via directories creating a separate output file per directory
        void get_image_input_output_params_feats(std::vector<std::vector<std::string> > &input_image_files, bool& as_video, std::vector<std::string> &arguments)
        {
            bool* valid = new bool[arguments.size()];

            for (size_t i = 0; i < arguments.size(); ++i)
            {
                valid[i] = true;
                if (arguments[i].compare("-fdir") == 0)
                {

                    // parse the -fdir directory by reading in all of the .png and .jpg files in it
                    boost::filesystem::path image_directory(arguments[i + 1]);

                    try
                    {
                        // does the file exist and is it a directory
                        if (exists(image_directory) && is_directory(image_directory))
                        {

                            std::vector<boost::filesystem::path> file_in_directory;
                            copy(boost::filesystem::directory_iterator(image_directory), boost::filesystem::directory_iterator(), back_inserter(file_in_directory));

                            // Sort the images in the directory first
                            sort(file_in_directory.begin(), file_in_directory.end());

                            std::vector<std::string> curr_dir_files;

                            for (std::vector<boost::filesystem::path>::const_iterator file_iterator(file_in_directory.begin()); file_iterator != file_in_directory.end(); ++file_iterator)
                            {
                                // Possible image extension .jpg and .png
                                if (file_iterator->extension().string().compare(".jpg") == 0 || file_iterator->extension().string().compare(".png") == 0)
                                {
                                    curr_dir_files.push_back(file_iterator->string());
                                }
                            }

                            input_image_files.push_back(curr_dir_files);
                        }
                    }
                    catch (const boost::filesystem::filesystem_error& ex)
                    {
                        std::cout << ex.what() << '\n';
                    }

                    valid[i] = false;
                    valid[i + 1] = false;
                    i++;
                }
                else if (arguments[i].compare("-asvid") == 0)
                {
                    as_video = true;
                }
            }

            // Clear up the argument list
            for (int i = arguments.size() - 1; i >= 0; --i)
            {
                if (!valid[i])
                {
                    arguments.erase(arguments.begin() + i);
                }
            }

        }

        void output_HOG_frame(std::ofstream* hog_file, bool good_frame, const cv::Mat_<double>& hog_descriptor, int num_rows, int num_cols)
        {

            // Using FHOGs, hence 31 channels
            int num_channels = 31;

            hog_file->write((char*)(&num_cols), 4);
            hog_file->write((char*)(&num_rows), 4);
            hog_file->write((char*)(&num_channels), 4);

            // Not the best way to store a bool, but will be much easier to read it
            float good_frame_float;
            if (good_frame)
                good_frame_float = 1;
            else
                good_frame_float = -1;

            hog_file->write((char*)(&good_frame_float), 4);

            cv::MatConstIterator_<double> descriptor_it = hog_descriptor.begin();

            for (int y = 0; y < num_cols; ++y)
            {
                for (int x = 0; x < num_rows; ++x)
                {
                    for (unsigned int o = 0; o < 31; ++o)
                    {

                        float hog_data = (float)(*descriptor_it++);
                        hog_file->write((char*)&hog_data, 4);
                    }
                }
            }
        }

        // Visualising the results
        void visualise_tracking(cv::Mat& captured_image, const LandmarkDetector::CLNF& face_model, const LandmarkDetector::FaceModelParameters& det_parameters, cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, int frame_count, double fx, double fy, double cx, double cy)
        {

            // Drawing the facial landmarks on the face and the bounding box around it if tracking is successful and initialised
            double detection_certainty = face_model.detection_certainty;
            bool detection_success = face_model.detection_success;

            double visualisation_boundary = 0.2;

            // Only draw if the reliability is reasonable, the value is slightly ad-hoc
            if (detection_certainty < visualisation_boundary)
            {
                LandmarkDetector::Draw(captured_image, face_model);

                double vis_certainty = detection_certainty;
                if (vis_certainty > 1)
                    vis_certainty = 1;
                if (vis_certainty < -1)
                    vis_certainty = -1;

                vis_certainty = (vis_certainty + 1) / (visualisation_boundary + 1);

                // A rough heuristic for box around the face width
                int thickness = (int)std::ceil(2.0* ((double)captured_image.cols) / 640.0);

                cv::Vec6d pose_estimate_to_draw = LandmarkDetector::GetCorrectedPoseWorld(face_model, fx, fy, cx, cy);

                // Draw it in reddish if uncertain, blueish if certain
                /*LandmarkDetector::DrawBox(captured_image, pose_estimate_to_draw, cv::Scalar((1 - vis_certainty)*255.0, 0, vis_certainty * 255), thickness, fx, fy, cx, cy);*/

                /*if (det_parameters.track_gaze && detection_success && face_model.eye_model)
                {
                    FaceAnalysis::DrawGaze(captured_image, face_model, gazeDirection0, gazeDirection1, fx, fy, cx, cy);
                }*/
            }

            // Work out the framerate
            if (frame_count % 10 == 0)
            {
                double t1 = cv::getTickCount();
                fps_tracker = 10.0 / (double(t1 - t0) / cv::getTickFrequency());
                t0 = t1;
            }

            // Write out the framerate on the image before displaying it
            char fpsC[255];
            std::sprintf(fpsC, "%d", (int)fps_tracker);
            std::string fpsSt("FPS:");
            fpsSt += fpsC;
            cv::putText(captured_image, fpsSt, cv::Point(10, 20), CV_FONT_HERSHEY_SIMPLEX, 0.5, CV_RGB(255, 0, 0), 1, CV_AA);

            if (!det_parameters.quiet_mode)
            {
                cv::namedWindow("tracking_result", 1);
                cv::imshow("tracking_result", captured_image);
            }
        }


        void prepareOutputFile(std::ofstream* output_file, bool output_2D_landmarks, bool output_3D_landmarks,
            bool output_model_params, bool output_pose, bool output_AUs, bool output_gaze,
            int num_landmarks, int num_model_modes, std::vector<std::string> au_names_class, std::vector<std::string> au_names_reg)
        {

            *output_file << "frame, timestamp, confidence, success";

            if (output_gaze)
            {
                *output_file << ", gaze_0_x, gaze_0_y, gaze_0_z, gaze_1_x, gaze_1_y, gaze_1_z";
            }

            if (output_pose)
            {
                *output_file << ", pose_Tx, pose_Ty, pose_Tz, pose_Rx, pose_Ry, pose_Rz";
            }

            if (output_2D_landmarks)
            {
                for (int i = 0; i < num_landmarks; ++i)
                {
                    *output_file << ", x_" << i;
                }
                for (int i = 0; i < num_landmarks; ++i)
                {
                    *output_file << ", y_" << i;
                }
            }

            if (output_3D_landmarks)
            {
                for (int i = 0; i < num_landmarks; ++i)
                {
                    *output_file << ", X_" << i;
                }
                for (int i = 0; i < num_landmarks; ++i)
                {
                    *output_file << ", Y_" << i;
                }
                for (int i = 0; i < num_landmarks; ++i)
                {
                    *output_file << ", Z_" << i;
                }
            }

            // Outputting model parameters (rigid and non-rigid), the first parameters are the 6 rigid shape parameters, they are followed by the non rigid shape parameters
            if (output_model_params)
            {
                *output_file << ", p_scale, p_rx, p_ry, p_rz, p_tx, p_ty";
                for (int i = 0; i < num_model_modes; ++i)
                {
                    *output_file << ", p_" << i;
                }
            }

            if (output_AUs)
            {
                std::sort(au_names_reg.begin(), au_names_reg.end());
                for (std::string reg_name : au_names_reg)
                {
                    *output_file << ", " << reg_name << "_r";
                }

                std::sort(au_names_class.begin(), au_names_class.end());
                for (std::string class_name : au_names_class)
                {
                    *output_file << ", " << class_name << "_c";
                }
            }

            *output_file << std::endl;

        }


        // Output all of the information into one file in one go (quite a few parameters, but simplifies the flow)
        void outputAllFeatures(std::ofstream* output_file, bool output_2D_landmarks, bool output_3D_landmarks,
            bool output_model_params, bool output_pose, bool output_AUs, bool output_gaze,
            const LandmarkDetector::CLNF& face_model, int frame_count, double time_stamp, bool detection_success,
            cv::Point3f gazeDirection0, cv::Point3f gazeDirection1, const cv::Vec6d& pose_estimate, double fx, double fy, double cx, double cy,
            const FaceAnalysis::FaceAnalyser& face_analyser)
        {

            double confidence = 0.5 * (1 - face_model.detection_certainty);

            *output_file << frame_count + 1 << ", " << time_stamp << ", " << confidence << ", " << detection_success;

            // Output the estimated gaze
            if (output_gaze)
            {
                *output_file << ", " << gazeDirection0.x << ", " << gazeDirection0.y << ", " << gazeDirection0.z
                    << ", " << gazeDirection1.x << ", " << gazeDirection1.y << ", " << gazeDirection1.z;
            }

            // Output the estimated head pose
            if (output_pose)
            {
                if (face_model.tracking_initialised)
                {
                    *output_file << ", " << pose_estimate[0] << ", " << pose_estimate[1] << ", " << pose_estimate[2]
                        << ", " << pose_estimate[3] << ", " << pose_estimate[4] << ", " << pose_estimate[5];
                }
                else
                {
                    *output_file << ", 0, 0, 0, 0, 0, 0";
                }
            }

            // Output the detected 2D facial landmarks
            if (output_2D_landmarks)
            {
                for (int i = 0; i < face_model.pdm.NumberOfPoints() * 2; ++i)
                {
                    if (face_model.tracking_initialised)
                    {
                        *output_file << ", " << face_model.detected_landmarks.at<double>(i);
                    }
                    else
                    {
                        *output_file << ", 0";
                    }
                }
            }

            // Output the detected 3D facial landmarks
            if (output_3D_landmarks)
            {
                cv::Mat_<double> shape_3D = face_model.GetShape(fx, fy, cx, cy);
                for (int i = 0; i < face_model.pdm.NumberOfPoints() * 3; ++i)
                {
                    if (face_model.tracking_initialised)
                    {
                        *output_file << ", " << shape_3D.at<double>(i);
                    }
                    else
                    {
                        *output_file << ", 0";
                    }
                }
            }

            if (output_model_params)
            {
                for (int i = 0; i < 6; ++i)
                {
                    if (face_model.tracking_initialised)
                    {
                        *output_file << ", " << face_model.params_global[i];
                    }
                    else
                    {
                        *output_file << ", 0";
                    }
                }
                for (int i = 0; i < face_model.pdm.NumberOfModes(); ++i)
                {
                    if (face_model.tracking_initialised)
                    {
                        *output_file << ", " << face_model.params_local.at<double>(i, 0);
                    }
                    else
                    {
                        *output_file << ", 0";
                    }
                }
            }



            if (output_AUs)
            {
                auto aus_reg = face_analyser.GetCurrentAUsReg();

                std::vector<std::string> au_reg_names = face_analyser.GetAURegNames();
                std::sort(au_reg_names.begin(), au_reg_names.end());

                // write out ar the correct index
                for (std::string au_name : au_reg_names)
                {
                    for (auto au_reg : aus_reg)
                    {
                        if (au_name.compare(au_reg.first) == 0)
                        {
                            *output_file << ", " << au_reg.second;
                            break;
                        }
                    }
                }

                if (aus_reg.size() == 0)
                {
                    for (size_t p = 0; p < face_analyser.GetAURegNames().size(); ++p)
                    {
                        *output_file << ", 0";
                    }
                }

                auto aus_class = face_analyser.GetCurrentAUsClass();

                std::vector<std::string> au_class_names = face_analyser.GetAUClassNames();
                std::sort(au_class_names.begin(), au_class_names.end());

                // write out ar the correct index
                for (std::string au_name : au_class_names)
                {
                    for (auto au_class : aus_class)
                    {
                        if (au_name.compare(au_class.first) == 0)
                        {
                            *output_file << ", " << au_class.second;
                            break;
                        }
                    }
                }

                if (aus_class.size() == 0)
                {
                    for (size_t p = 0; p < face_analyser.GetAUClassNames().size(); ++p)
                    {
                        *output_file << ", 0";
                    }
                }
            }
            *output_file << std::endl;
        }

        // Allows for post processing of the AU signal
        void post_process_output_file(FaceAnalysis::FaceAnalyser& face_analyser, std::string output_file, bool dynamic)
        {

            std::vector<double> certainties;
            std::vector<bool> successes;
            std::vector<double> timestamps;
            std::vector<std::pair<std::string, std::vector<double>>> predictions_reg;
            std::vector<std::pair<std::string, std::vector<double>>> predictions_class;

            // Construct the new values to overwrite the output file with
            face_analyser.ExtractAllPredictionsOfflineReg(predictions_reg, certainties, successes, timestamps, dynamic);
            face_analyser.ExtractAllPredictionsOfflineClass(predictions_class, certainties, successes, timestamps, dynamic);

            int num_class = predictions_class.size();
            int num_reg = predictions_reg.size();

            // Extract the indices of writing out first
            std::vector<std::string> au_reg_names = face_analyser.GetAURegNames();
            std::sort(au_reg_names.begin(), au_reg_names.end());
            std::vector<int> inds_reg;

            // write out ar the correct index
            for (std::string au_name : au_reg_names)
            {
                for (int i = 0; i < num_reg; ++i)
                {
                    if (au_name.compare(predictions_reg[i].first) == 0)
                    {
                        inds_reg.push_back(i);
                        break;
                    }
                }
            }

            std::vector<std::string> au_class_names = face_analyser.GetAUClassNames();
            std::sort(au_class_names.begin(), au_class_names.end());
            std::vector<int> inds_class;

            // write out ar the correct index
            for (std::string au_name : au_class_names)
            {
                for (int i = 0; i < num_class; ++i)
                {
                    if (au_name.compare(predictions_class[i].first) == 0)
                    {
                        inds_class.push_back(i);
                        break;
                    }
                }
            }
            // Read all of the output file in
            std::vector<std::string> output_file_contents;

            std::ifstream infile(output_file);
            std::string line;

            while (std::getline(infile, line))
                output_file_contents.push_back(line);

            infile.close();

            // Read the header and find all _r and _c parts in a file and use their indices
            std::vector<std::string> tokens;
            boost::split(tokens, output_file_contents[0], boost::is_any_of(","));

            int begin_ind = -1;

            for (size_t i = 0; i < tokens.size(); ++i)
            {
                if (tokens[i].find("AU") != std::string::npos && begin_ind == -1)
                {
                    begin_ind = i;
                    break;
                }
            }
            int end_ind = begin_ind + num_class + num_reg;

            // Now overwrite the whole file
            std::ofstream outfile(output_file, ios_base::out);
            // Write the header
            outfile << output_file_contents[0].c_str() << std::endl;

            // Write the contents
            for (int i = 1; i < (int)output_file_contents.size(); ++i)
            {
                std::vector<std::string> tokens;
                boost::split(tokens, output_file_contents[i], boost::is_any_of(","));

                outfile << tokens[0];

                for (int t = 1; t < (int)tokens.size(); ++t)
                {
                    if (t >= begin_ind && t < end_ind)
                    {
                        if (t - begin_ind < num_reg)
                        {
                            outfile << ", " << predictions_reg[inds_reg[t - begin_ind]].second[i - 1];
                        }
                        else
                        {
                            outfile << ", " << predictions_class[inds_class[t - begin_ind - num_reg]].second[i - 1];
                        }
                    }
                    else
                    {
                        outfile << ", " << tokens[t];
                    }
                }
                outfile << std::endl;
            }


        }

        void lowpassFilter(std::vector<double>& auForTransmission) {
            int templateLength = 5;
            std::vector<double> templateWeights = { 0.2, 0.2, 0.2, 0.2, 0.2/*, 0.1, 0.1, 0.1, 0.1, 0.1*/ };
            for (size_t i = 0; i < CFeatureExtraction::m_datas.size(); i++) {
                CFeatureExtraction::m_datas[i][QUEUE_LENGTH_MAX - 1] = CFeatureExtraction::m_datas[i][QUEUE_LENGTH_MAX - 1] * templateWeights[0];
                for (int j = 1; j < templateLength; j++) {
                    CFeatureExtraction::m_datas[i][QUEUE_LENGTH_MAX - 1] += CFeatureExtraction::m_datas[i][QUEUE_LENGTH_MAX - j - 1] * templateWeights[j];
                }
                auForTransmission[i] = CFeatureExtraction::m_datas[i][QUEUE_LENGTH_MAX - 1];
            }
        }

        const std::vector<std::vector<double>>& CFeatureExtraction::GetDatas() const {
            return m_datas;
        }

        void CFeatureExtraction::Start()
        {
            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);

            ::SetEvent(sm_hEvents[EVT_CAMERA_RUNNING]);
            sm_nStatus |= STATUS_RUNNING;
        }

        void CFeatureExtraction::Pause()
        {
            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);

            sm_nStatus |= STATUS_PAUSE;
        }

        void CFeatureExtraction::Resume()
        {
            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);

            sm_nStatus &= ~STATUS_PAUSE;
            sm_nStatus |= STATUS_NEED_BASE_LINE;
        }

        void CFeatureExtraction::Quit()
        {
            Poco::ScopedLock<Poco::Mutex> lock(sm_statusMutex);

            ::SetEvent(sm_hEvents[EVT_CAMERA_QUIT]);
            sm_nStatus |= STATUS_QUIT;
        }

        Poco::BasicEvent<const TCameraData> CFeatureExtraction::OnDataUpdate;
        Poco::BasicEvent<const size_t> CFeatureExtraction::OnFpsUpdate;

    } // Camera

} // MyWeb