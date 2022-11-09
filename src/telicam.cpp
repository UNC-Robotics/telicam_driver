#include <iostream>

#include "telicam.hpp"

// Static variables
bool TeliCam::api_initialized = false;
Teli::CAM_SYSTEM_INFO TeliCam::sys_info = Teli::CAM_SYSTEM_INFO();
uint32_t TeliCam::num_cameras = 0;

TeliCam::TeliCam(int camera_index) : cam_id(camera_index), camera_initialized(false)
{
}

void TeliCam::initialize(const Parameters &parameters)
{
    if (camera_initialized)
    {
        close_camera_stream();
        close_camera();
    }

    get_system_info();
    get_num_cameras();
    get_camera_info();

    open_camera();
    get_camera_parameter_limits();
    set_camera_parameters(parameters);
    get_camera_properties();
}

void TeliCam::start()
{
    open_camera_stream();
    start_stream();
}

void TeliCam::stop()
{
    stop_stream();
    close_camera_stream();
}

void TeliCam::destroy()
{
    close_camera_stream();
    close_camera();
}

cv::Mat TeliCam::get_last_frame()
{
    // last_frame_data.frame_mutex.lock();
    return last_frame_data.frame.clone();
    // last_frame_data.frame_mutex.unlock();
}

TeliCam::Parameters TeliCam::get_parameters() const
{
    return parameters;
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
    // Exposure
    Teli::GetCamExposureTimeMinMax(cam_handle, &min_exposure_time, &max_exposure_time);

    // Saturation
    Teli::GetCamSaturationMinMax(cam_handle, &min_saturation, &max_saturation);

    // Gamma
    Teli::GetCamGammaMinMax(cam_handle, &min_gamma, &max_gamma);

    // Hue
    Teli::GetCamHueMinMax(cam_handle, &min_hue, &max_hue);

    // Gain
    Teli::GetCamGainMinMax(cam_handle, &min_gain, &max_gain);

    // Framerate
    Teli::GetCamAcquisitionFrameRateMinMax(cam_handle, &min_framerate, &max_framerate);
    std::cout << "Framerate min: " << min_framerate << " max: " << max_framerate << std::endl;
}

void TeliCam::set_camera_parameters(Parameters parameters)
{
    Teli::SetCamExposureTimeControl(cam_handle, Teli::CAM_EXPOSURE_TIME_CONTROL_MANUAL);
    Teli::SetCamAcquisitionFrameRateControl(cam_handle, Teli::CAM_ACQ_FRAME_RATE_CTRL_MANUAL);

    // Exposure
    if (parameters.exposure_time <= max_exposure_time && parameters.exposure_time >= min_exposure_time)
    {
        Teli::SetCamExposureTime(cam_handle, parameters.exposure_time);
    }
    else
    {
        throw std::runtime_error("Exposure time out of range");
    }

    // Saturation
    if (parameters.saturation <= max_saturation && parameters.saturation >= min_saturation)
    {
        Teli::SetCamSaturation(cam_handle, parameters.saturation);
    }
    else
    {
        throw std::runtime_error("Saturation out of range");
    }

    // Gamma
    if (parameters.gamma <= max_gamma && parameters.gamma >= min_gamma)
    {
        Teli::SetCamGamma(cam_handle, parameters.gamma);
    }
    else
    {
        throw std::runtime_error("Gamma out of range");
    }

    // Hue
    if (parameters.hue <= max_hue && parameters.hue >= min_hue)
    {
        Teli::SetCamHue(cam_handle, parameters.hue);
    }
    else
    {
        throw std::runtime_error("Hue out of range");
    }

    // Gain
    if (parameters.gain <= max_gain && parameters.gain >= min_gain)
    {
        Teli::SetCamGain(cam_handle, parameters.gain);
    }
    else
    {
        throw std::runtime_error("Gain out of range");
    }

    // Framerate
    if (parameters.framerate <= max_framerate && parameters.framerate >= min_framerate)
    {
        Teli::SetCamAcquisitionFrameRate(cam_handle, parameters.framerate);
    }
    else
    {
        throw std::runtime_error("Framerate out of range");
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
    Teli::GetCamSensorWidth(cam_handle, &width);
    Teli::GetCamSensorHeight(cam_handle, &height);
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

    TeliCam::LastFrameData *last_frame_data = reinterpret_cast<TeliCam::LastFrameData *>(pvContext);
    // last_frame_data->frame_mutex.lock();
    last_frame_data->frame = image.clone();
    // last_frame_data->frame_mutex.unlock();
}

void TeliCam::open_camera_stream()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_OpenSimple(cam_handle, &cam_stream_handle, &image_buffer_size);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_OpenSimple failed");
    }

    void *last_frame_data_ptr = reinterpret_cast<void *>(&last_frame_data);
    cam_status = Teli::Strm_SetCallbackImageAcquired(cam_stream_handle, last_frame_data_ptr, CallbackImageAcquired);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_SetCallbackImageAcquired failed");
    }
}

void TeliCam::start_stream()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_Start(cam_stream_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Start failed");
    }
}

void TeliCam::stop_stream()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_Stop(cam_stream_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Stop failed");
    }
}

void TeliCam::close_camera_stream()
{
    Teli::CAM_API_STATUS cam_status = Teli::Strm_Close(cam_stream_handle);
    if (cam_status != Teli::CAM_API_STS_SUCCESS)
    {
        throw std::runtime_error("Telicam Strm_Close failed");
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
