#pragma once

#include <opencv2/core/core.hpp>

#include <TeliCamApi.h>
#include <TeliCamUtl.h>

class TeliCam
{
  public:
    struct Parameters
    {
        float64_t exposure_time = 25000.0f;
        float64_t saturation = 100.0f;
        float64_t gamma = 1.0f;
        float64_t hue = 0.0f;
        float64_t gain = 0.0f;
        float64_t framerate = 20.0f;
        bool auto_white_balance = true;
        bool auto_gain = true;
        bool trigger_mode = false;
    };

    struct LastFrameData
    {
        std::mutex frame_mutex;
        cv::Mat frame;
    };

  public:
    explicit TeliCam(int camera_index);

    // These must be called at least once
    static void initialize_api();
    static void close_api();

    void initialize(const Parameters &parameters);

    void start();

    void stop();

    void destroy();

    cv::Mat get_last_frame();

    Parameters get_parameters() const;

    void print_camera_info() const;

  private:
    void get_system_info();
    void get_num_cameras();
    void get_camera_info();
    void open_camera();
    void get_camera_parameter_limits();
    void set_camera_parameters(Parameters parameters);
    void get_camera_properties();
    void open_camera_stream();
    void start_stream();
    void stop_stream();
    void close_camera_stream();
    void close_camera();

  private:
    static bool api_initialized;
    static Teli::CAM_SYSTEM_INFO sys_info;
    static uint32_t num_cameras;

    bool camera_initialized;

    uint32_t cam_id;
    Teli::CAM_INFO cam_info;

    Teli::CAM_HANDLE cam_handle;
    Teli::CAM_STRM_HANDLE cam_stream_handle;

    uint32_t width;
    uint32_t height;
    float64_t framerate;
    uint32_t image_buffer_size;

    Parameters parameters;
    float64_t min_exposure_time;
    float64_t max_exposure_time;
    float64_t min_saturation;
    float64_t max_saturation;
    float64_t min_gamma;
    float64_t max_gamma;
    float64_t min_hue;
    float64_t max_hue;
    float64_t min_gain;
    float64_t max_gain;
    float64_t min_framerate;
    float64_t max_framerate;

    LastFrameData last_frame_data;
};
