#pragma once

#include <opencv2/core/core.hpp>

#include <TeliCamApi.h>
#include <TeliCamUtl.h>

/**
 * @brief Driver for controlling Toshiba TeliCams
 */
class TeliCam
{
  public:
    struct Parameters
    {
        uint32_t width = 0; // Default is max width
        uint32_t height = 0; // Default is max height
        uint32_t offset_x = 0;
        uint32_t offset_y = 0;

        uint32_t binning_x = 0;
        uint32_t binning_y = 0;
        uint32_t decimation_x = 0;
        uint32_t decimation_y = 0;

        float64_t exposure_time = 25000.0f;
        float64_t saturation = 100.0f;
        float64_t gamma = 1.0f;
        float64_t hue = 0.0f;
        float64_t gain = 0.0f;
        bool auto_gain = false;
        float64_t black_level = 0.0f;
        float64_t framerate = 30.0f;
        uint32_t sharpness = 0;
        float64_t balance_ratio_r = 4.0f;
        float64_t balance_ratio_b = 4.0f;
        bool auto_white_balance = true;

        bool reverse_x = false;
        bool reverse_y = false;
        bool trigger_mode = false;
    };

    struct SupportedFeatures
    {
      bool has_binning;
      bool has_decimation;
      bool has_exposure_time;
      bool has_saturation;
      bool has_gamma;
      bool has_hue;
      bool has_gain;
      bool has_auto_gain;
      bool has_black_level;
      bool has_framerate;
      bool has_sharpness;
      bool has_reverse_x;
      bool has_reverse_y;
      bool has_balance_ratio_r;
      bool has_balance_ratio_b;
    };

  public:
    explicit TeliCam(int camera_index);

    /**
     * @brief Initialize the TeliCam API. Must be called once per program.
     */
    static void initialize_api();

    /**
     * @brief Close the TeliCam API. Must be called once per program.
     */
    static void close_api();

    /**
     * @brief Initialize the TeliCam with the given parameters.
     * 
     * @param parameters TeliCam parameters
     */
    void initialize(const Parameters &parameters);

    /**
     * @brief Start continuous streaming from the TeliCam.
     */ 
    void start_stream();

    /**
     * @brief Capture a single frame from the TeliCam. There must be no active stream.
     */
    void capture_frame();

    /**
     * @brief Stop continuous streaming from the TeliCam.
     * 
     */
    void stop_stream();

    /**
     * @brief Destroy the TeliCam. This will close the stream and camera. Does not close the API.
     */
    void destroy();

    /**
     * @brief Get the last captured frame from either continuous streaming or a single capture.
     * 
     * @return cv::Mat Last captured frame in OpenCV format
     */
    cv::Mat get_last_frame();

    /**
     * @brief Get the TeliCam parameters.
     * 
     * @return Parameters TeliCam parameters.
     */
    Parameters get_parameters() const;

    /**
     * @brief Print TeliCam API system information.
     */
    void print_system_info() const;

    /**
     * @brief Print TeliCam information.
     */
    void print_camera_info() const;

  private:
    void get_system_info();
    void get_num_cameras();
    void get_camera_info();
    void open_camera();
    void get_camera_parameter_limits();
    void set_camera_parameters(Parameters parameters);
    void get_camera_properties();
    void open_stream();
    void capture_frame_internal();
    void start_stream_internal();
    void stop_stream_internal();
    void close_camera();

  private:
    static bool api_initialized;
    static Teli::CAM_SYSTEM_INFO sys_info;
    static uint32_t num_cameras;

    bool camera_initialized;
    bool camera_stream_opened;

    uint32_t cam_id;
    Teli::CAM_INFO cam_info;

    Teli::CAM_HANDLE cam_handle;
    Teli::CAM_STRM_HANDLE cam_stream_handle;

    uint32_t width;
    uint32_t height;
    uint32_t sensor_width;    
    uint32_t sensor_height;
    float64_t framerate;
    uint32_t image_buffer_size;

    Parameters parameters;
    SupportedFeatures features;

    uint32_t min_width;
    uint32_t max_width;
    uint32_t width_inc;
    uint32_t min_height;
    uint32_t max_height;
    uint32_t height_inc;
    uint32_t min_offset_x;
    uint32_t max_offset_x;
    uint32_t offset_x_inc;
    uint32_t min_offset_y;
    uint32_t max_offset_y;
    uint32_t offset_y_inc;
    uint32_t min_binning_x;
    uint32_t max_binning_x;
    uint32_t min_binning_y;
    uint32_t max_binning_y;
    uint32_t min_decimation_x;
    uint32_t max_decimation_x;
    uint32_t min_decimation_y;
    uint32_t max_decimation_y;

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
    float64_t min_black_level;
    float64_t max_black_level;
    float64_t min_framerate;
    float64_t max_framerate;
    uint32_t min_sharpness;
    uint32_t max_sharpness;
    float64_t min_balance_ratio_r;
    float64_t max_balance_ratio_r;
    float64_t min_balance_ratio_b;
    float64_t max_balance_ratio_b;

    cv::Mat last_frame;
};
