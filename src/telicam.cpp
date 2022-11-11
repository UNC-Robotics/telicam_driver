#include <iostream>

#include "telicam.hpp"

// Static variables
bool TeliCam::api_initialized = false;
Teli::CAM_SYSTEM_INFO TeliCam::sys_info = Teli::CAM_SYSTEM_INFO();
uint32_t TeliCam::num_cameras = 0;

TeliCam::TeliCam() : cam_id(0), camera_initialized(false), streaming(false)
{
}

TeliCam::TeliCam(int camera_index)
    : cam_id(camera_index), camera_initialized(false), streaming(false)
{
}

void TeliCam::initialize(const Parameters &parameters)
{
    if (camera_initialized)
    {
        close_camera();
    }

    get_system_info();
    get_num_cameras();
    get_camera_info();

    open_camera();
    get_camera_parameter_limits();
    set_camera_parameters(parameters);
    get_camera_properties();
    open_stream();

    // Allocate all black image to last_frame
    last_frame = cv::Mat(height, width, CV_8UC1, cv::Scalar(0));
}

void TeliCam::start_stream()
{
    if (streaming)
        return;

    start_stream_internal();
    streaming = true;
}

void TeliCam::capture_frame()
{
    capture_frame_internal();
}

void TeliCam::stop_stream()
{
    if (!streaming)
        return;

    stop_stream_internal();
}

void TeliCam::destroy()
{
    if (streaming)
    {
        TeliCam::stop_stream();
    }

    close_camera();
}

bool TeliCam::is_streaming() const
{
    return streaming;
}

cv::Mat TeliCam::get_last_frame()
{
    return last_frame.clone();
}

TeliCam::Parameters TeliCam::get_parameters() const
{
    return parameters;
}

TeliCam::SupportedFeatures TeliCam::get_supported_features() const
{
    return features;
}

void TeliCam::print_system_info() const
{
    std::cout << "TeliCam API System info:" << std::endl;
    std::cout << "  Driver version: " << sys_info.sU3vInfo.szDriverVersion << std::endl;
    std::cout << "  API version: " << sys_info.sU3vInfo.szDllVersion << std::endl;
    std::cout << "  Number of cameras: " << num_cameras << std::endl;
}

void TeliCam::print_camera_info() const
{
    std::cout << "TeliCam information:" << std::endl;
    std::cout << "  Camera ID: " << cam_id << std::endl;
    std::cout << "  Camera manufacturer: " << cam_info.szManufacturer << std::endl;
    std::cout << "  Camera model: " << cam_info.szModelName << std::endl;
    std::cout << "  Camera serial number: " << cam_info.szSerialNumber << std::endl;
}

void TeliCam::print_parameters() const
{
    std::cout << "TeliCam parameters:" << std::endl;
    std::cout << "  Width: " << parameters.width << std::endl;
    std::cout << "  Height: " << parameters.height << std::endl;
    std::cout << "  Offset X: " << parameters.offset_x << std::endl;
    std::cout << "  Offset Y: " << parameters.offset_y << std::endl;
    std::cout << "  Binning X: " << parameters.binning_x << std::endl;
    std::cout << "  Binning Y: " << parameters.binning_y << std::endl;
    std::cout << "  Decimation X: " << parameters.decimation_x << std::endl;
    std::cout << "  Decimation Y: " << parameters.decimation_y << std::endl;
    std::cout << "  Exposure time: " << parameters.exposure_time << std::endl;
    std::cout << "  Saturation: " << parameters.saturation << std::endl;
    std::cout << "  Gamma: " << parameters.gamma << std::endl;
    std::cout << "  Hue: " << parameters.hue << std::endl;
    std::cout << "  Gain: " << parameters.gain << std::endl;
    std::cout << "  Auto gain: " << parameters.auto_gain << std::endl;
    std::cout << "  Black level: " << parameters.black_level << std::endl;
    std::cout << "  Framerate: " << parameters.framerate << std::endl;
    std::cout << "  Sharpness: " << parameters.sharpness << std::endl;
    std::cout << "  Balance ratio R: " << parameters.balance_ratio_r << std::endl;
    std::cout << "  Balance ratio B: " << parameters.balance_ratio_b << std::endl;
    std::cout << "  Auto white balance: " << parameters.auto_white_balance << std::endl;
    std::cout << "  Reverse X: " << parameters.reverse_x << std::endl;
    std::cout << "  Reverse Y: " << parameters.reverse_y << std::endl;
    std::cout << "  Trigger mode: " << parameters.trigger_mode << std::endl;
}

void TeliCam::initialize_api()
{
    if (api_initialized)
        return;

    Teli::CAM_API_STATUS cam_status = Teli::Sys_Initialize();
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Sys_Initialize failed");
    }
}

void TeliCam::get_system_info()
{
    Teli::CAM_API_STATUS cam_status = Teli::Sys_GetInformation(&TeliCam::sys_info);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Sys_GetInformation failed");
    }
}

void TeliCam::get_num_cameras()
{
    Teli::CAM_API_STATUS cam_status = Teli::Sys_GetNumOfCameras(&TeliCam::num_cameras);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Sys_GetNumOfCameras failed");
    }
}

void TeliCam::get_camera_info()
{
    Teli::CAM_API_STATUS cam_status = Teli::Cam_GetInformation((Teli::CAM_HANDLE)NULL, cam_id, &cam_info);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Cam_GetInformation failed");
    }

    if (cam_info.eCamType != Teli::CAM_TYPE_U3V)
    {
        throw std::runtime_error("Only USB3 Telicams are supported");
    }
}

void TeliCam::open_camera()
{
    Teli::CAM_API_STATUS cam_status = Teli::Cam_Open(cam_id, &cam_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Cam_Open failed");
    }
}

void TeliCam::get_camera_parameter_limits()
{
    Teli::CAM_API_STATUS cam_status;

    // Width
    Teli::GetCamWidthMinMax(cam_handle, &min_width, &max_width, &width_inc);

    // Height
    Teli::GetCamHeightMinMax(cam_handle, &min_height, &max_height, &height_inc);

    // Offset
    Teli::GetCamOffsetXMinMax(cam_handle, &min_offset_x, &max_offset_x, &offset_x_inc);
    Teli::GetCamOffsetYMinMax(cam_handle, &min_offset_y, &max_offset_y, &offset_y_inc);

    // Binning
    cam_status = Teli::GetCamBinningHorizontalMinMax(cam_handle, &min_binning_x, &max_binning_x);
    cam_status = Teli::GetCamBinningVerticalMinMax(cam_handle, &min_binning_y, &max_binning_y);
    features.has_binning = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Decimation
    cam_status = Teli::GetCamDecimationHorizontalMinMax(cam_handle, &min_decimation_x, &max_decimation_x);
    cam_status = Teli::GetCamDecimationVerticalMinMax(cam_handle, &min_decimation_y, &max_decimation_y);
    features.has_decimation = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Exposure
    cam_status = Teli::GetCamExposureTimeMinMax(cam_handle, &min_exposure_time, &max_exposure_time);
    features.has_exposure_time = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Saturation
    cam_status = Teli::GetCamSaturationMinMax(cam_handle, &min_saturation, &max_saturation);
    features.has_saturation = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Gamma
    cam_status = Teli::GetCamGammaMinMax(cam_handle, &min_gamma, &max_gamma);
    features.has_gamma = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Hue
    cam_status = Teli::GetCamHueMinMax(cam_handle, &min_hue, &max_hue);
    features.has_hue = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Gain
    cam_status = Teli::GetCamGainMinMax(cam_handle, &min_gain, &max_gain);
    features.has_gain = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Black level
    cam_status = Teli::GetCamBlackLevelMinMax(cam_handle, &min_black_level, &max_black_level);
    features.has_black_level = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Framerate
    cam_status = Teli::GetCamAcquisitionFrameRateMinMax(cam_handle, &min_framerate, &max_framerate);
    features.has_framerate = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Sharpness
    cam_status = Teli::GetCamSharpnessMinMax(cam_handle, &min_sharpness, &max_sharpness);
    features.has_sharpness = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Balance ratio R
    cam_status = Teli::GetCamBalanceRatioMinMax(cam_handle, Teli::CAM_BALANCE_RATIO_SELECTOR_RED, &min_balance_ratio_r,
                                                &max_balance_ratio_r);
    features.has_balance_ratio_r = (cam_status == Teli::CAM_API_STS_SUCCESS);

    // Balance ratio B
    cam_status = Teli::GetCamBalanceRatioMinMax(cam_handle, Teli::CAM_BALANCE_RATIO_SELECTOR_BLUE, &min_balance_ratio_b,
                                                &max_balance_ratio_b);
    features.has_balance_ratio_b = (cam_status == Teli::CAM_API_STS_SUCCESS);
}

void TeliCam::set_camera_parameters(Parameters parameters)
{
    Teli::SetCamExposureTimeControl(cam_handle, Teli::CAM_EXPOSURE_TIME_CONTROL_MANUAL);
    Teli::SetCamAcquisitionFrameRateControl(cam_handle, Teli::CAM_ACQ_FRAME_RATE_CTRL_MANUAL);

    // Width
    if (parameters.width == 0)
    {
        parameters.width = max_width;
    }
    if (parameters.width <= max_width && parameters.width >= min_width)
    {
        Teli::SetCamWidth(cam_handle, parameters.width);
    }
    else
    {
        throw std::runtime_error("Width out of range");
    }

    // Height
    if (parameters.height == 0)
    {
        parameters.height = max_height;
    }
    if (parameters.height <= max_height && parameters.height >= min_height)
    {
        Teli::SetCamHeight(cam_handle, parameters.height);
    }
    else
    {
        throw std::runtime_error("Height out of range");
    }

    // Offset X
    if (parameters.offset_x <= max_offset_x && parameters.offset_x >= min_offset_x)
    {
        Teli::SetCamOffsetX(cam_handle, parameters.offset_x);
    }
    else
    {
        throw std::runtime_error("Offset X out of range");
    }

    // Offset Y
    if (parameters.offset_y <= max_offset_y && parameters.offset_y >= min_offset_y)
    {
        Teli::SetCamOffsetY(cam_handle, parameters.offset_y);
    }
    else
    {
        throw std::runtime_error("Offset Y out of range");
    }

    // Binning
    if (features.has_binning)
    {
        if (parameters.binning_x <= max_binning_x && parameters.binning_x >= min_binning_x)
        {
            Teli::SetCamBinningHorizontal(cam_handle, parameters.binning_x);
        }
        else
        {
            throw std::runtime_error("Binning X out of range");
        }

        if (parameters.binning_y <= max_binning_y && parameters.binning_y >= min_binning_y)
        {
            Teli::SetCamBinningVertical(cam_handle, parameters.binning_y);
        }
        else
        {
            throw std::runtime_error("Binning Y out of range");
        }
    }

    // Decimation
    if (features.has_decimation)
    {
        if (parameters.decimation_x <= max_decimation_x && parameters.decimation_x >= min_decimation_x)
        {
            Teli::SetCamDecimationHorizontal(cam_handle, parameters.decimation_x);
        }
        else
        {
            throw std::runtime_error("Decimation X out of range");
        }

        if (parameters.decimation_y <= max_decimation_y && parameters.decimation_y >= min_decimation_y)
        {
            Teli::SetCamDecimationVertical(cam_handle, parameters.decimation_y);
        }
        else
        {
            throw std::runtime_error("Decimation Y out of range");
        }
    }

    // Exposure time
    if (features.has_exposure_time)
    {
        if (parameters.exposure_time <= max_exposure_time && parameters.exposure_time >= min_exposure_time)
        {
            Teli::SetCamExposureTime(cam_handle, parameters.exposure_time);
        }
        else
        {
            throw std::runtime_error("Exposure time out of range");
        }
    }

    // Saturation
    if (features.has_saturation)
    {
        if (parameters.saturation <= max_saturation && parameters.saturation >= min_saturation)
        {
            Teli::SetCamSaturation(cam_handle, parameters.saturation);
        }
        else
        {
            throw std::runtime_error("Saturation out of range");
        }
    }

    // Gamma
    if (features.has_gamma)
    {
        if (parameters.gamma <= max_gamma && parameters.gamma >= min_gamma)
        {
            Teli::SetCamGamma(cam_handle, parameters.gamma);
        }
        else
        {
            throw std::runtime_error("Gamma out of range");
        }
    }

    // Hue
    if (features.has_hue)
    {
        if (parameters.hue <= max_hue && parameters.hue >= min_hue)
        {
            Teli::SetCamHue(cam_handle, parameters.hue);
        }
        else
        {
            throw std::runtime_error("Hue out of range");
        }
    }

    // Gain
    if (features.has_gain)
    {
        if (parameters.gain <= max_gain && parameters.gain >= min_gain)
        {
            Teli::SetCamGain(cam_handle, parameters.gain);
        }
        else
        {
            throw std::runtime_error("Gain out of range");
        }
    }

    // Black level
    if (features.has_black_level)
    {
        if (parameters.black_level <= max_black_level && parameters.black_level >= min_black_level)
        {
            Teli::SetCamBlackLevel(cam_handle, parameters.black_level);
        }
        else
        {
            throw std::runtime_error("Black level out of range");
        }
    }

    // Framerate
    if (features.has_framerate)
    {
        if (parameters.framerate <= max_framerate && parameters.framerate >= min_framerate)
        {
            Teli::SetCamAcquisitionFrameRate(cam_handle, parameters.framerate);
        }
        else
        {
            throw std::runtime_error("Framerate out of range");
        }
    }

    // Sharpness
    if (features.has_sharpness)
    {
        if (parameters.sharpness <= max_sharpness && parameters.sharpness >= min_sharpness)
        {
            Teli::SetCamSharpness(cam_handle, parameters.sharpness);
        }
        else
        {
            throw std::runtime_error("Sharpness out of range");
        }
    }

    // Reverse
    if (features.has_reverse_x)
    {
        Teli::SetCamReverseX(cam_handle, parameters.reverse_x);
    }
    if (features.has_reverse_y)
    {
        Teli::SetCamReverseY(cam_handle, parameters.reverse_y);
    }

    // Balance ratio R
    if (features.has_balance_ratio_r)
    {
        if (parameters.balance_ratio_r <= max_balance_ratio_r && parameters.balance_ratio_r >= min_balance_ratio_r)
        {
            Teli::SetCamBalanceRatio(cam_handle, Teli::CAM_BALANCE_RATIO_SELECTOR_RED, parameters.balance_ratio_r);
        }
        else
        {
            throw std::runtime_error("Balance ratio R out of range");
        }
    }

    // Balance ratio B
    if (features.has_balance_ratio_b)
    {
        if (parameters.balance_ratio_b <= max_balance_ratio_b && parameters.balance_ratio_b >= min_balance_ratio_b)
        {
            Teli::SetCamBalanceRatio(cam_handle, Teli::CAM_BALANCE_RATIO_SELECTOR_BLUE, parameters.balance_ratio_b);
        }
        else
        {
            throw std::runtime_error("Balance ratio B out of range");
        }
    }

    // Auto-white balance
    if (parameters.auto_white_balance)
    {
        Teli::SetCamBalanceWhiteAuto(cam_handle, Teli::CAM_BALANCE_WHITE_AUTO_ONCE);
    }
    else
    {
        Teli::SetCamBalanceWhiteAuto(cam_handle, Teli::CAM_BALANCE_WHITE_AUTO_OFF);
    }

    // Auto-gain
    if (parameters.auto_gain)
    {
        Teli::SetCamGainAuto(cam_handle, Teli::CAM_GAIN_AUTO_AUTO);
    }
    else
    {
        Teli::SetCamGainAuto(cam_handle, Teli::CAM_GAIN_AUTO_OFF);
    }

    // Trigger mode
    Teli::SetCamTriggerMode(cam_handle, parameters.trigger_mode);
}

void TeliCam::get_camera_properties()
{
    Teli::GetCamWidth(cam_handle, &width);
    Teli::GetCamHeight(cam_handle, &height);
    Teli::GetCamSensorWidth(cam_handle, &sensor_width);
    Teli::GetCamSensorHeight(cam_handle, &sensor_height);
    Teli::GetCamAcquisitionFrameRate(cam_handle, &framerate);
}

void CallbackImageAcquired(Teli::CAM_HANDLE cam_handle, Teli::CAM_STRM_HANDLE cam_stream_handle,
                           Teli::CAM_IMAGE_INFO *image_info, uint32_t buffer_index, void *pvContext)
{
    uint8_t *image_buffer = (uint8_t *)image_info->pvBuf;

    uint32_t image_width = image_info->uiSizeX;
    uint32_t image_height = image_info->uiSizeY;
    cv::Mat image(cv::Size(image_width, image_height), CV_8UC3, cv::Scalar::all(0));

    Teli::ConvImage(Teli::DST_FMT_BGR24, image_info->uiPixelFormat, true, image.data, image_buffer, image_width,
                    image_height);

    cv::Mat *last_frame = reinterpret_cast<cv::Mat *>(pvContext);
    *last_frame = image.clone();
}

void TeliCam::open_stream()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_OpenSimple(cam_handle, &cam_stream_handle, &image_buffer_size);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_OpenSimple failed");
    }

    void *last_frame_ptr = reinterpret_cast<void *>(&last_frame);
    cam_status = Teli::Strm_SetCallbackImageAcquired(cam_stream_handle, last_frame_ptr, CallbackImageAcquired);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_SetCallbackImageAcquired failed");
    }

    // Print sensor width and height
    std::cout << "Sensor width: " << sensor_width << std::endl;
    std::cout << "Sensor height: " << sensor_height << std::endl;
}

void TeliCam::capture_frame_internal()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_Start(cam_stream_handle, Teli::CAM_ACQ_MODE_SINGLE_FRAME);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Start single-frame failed");
    }
}

void TeliCam::start_stream_internal()
{

    Teli::CAM_API_STATUS cam_status = Teli::Strm_Start(cam_stream_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Start failed");
    }
}

void TeliCam::stop_stream_internal()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_Stop(cam_stream_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Stop failed");
    }
}

void TeliCam::close_camera()
{
    Teli::CAM_API_STATUS cam_status = Teli::Cam_Close(cam_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam camera close failed");
    }
}

void TeliCam::close_api()
{
    Teli::CAM_API_STATUS cam_status = Teli::Sys_Terminate();
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam API close failed");
    }
}
