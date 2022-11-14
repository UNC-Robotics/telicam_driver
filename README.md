# Toshiba TeliCam Driver
This is a high-level driver for Toshiba USB3 TeliCams. It uses the TeliCamSDK and offers a simple interface.

Included is `telicam_viewer`, an application for viewing TeliCam video feeds.

## Requirements
* TeliCamSDK
* OpenCV

## API Usage
Here is a minimal example to video from the first TeliCam connected to the system:
```cpp
#include <thread>
#include <opencv2/highgui/highgui.hpp>

#include <telicam.hpp>

int main(int argc, char** argv)
{
    // Open and configure TeliCam ///////////////////////////////////
    TeliCam::initialize_api();

    int cam_id = 0;
    TeliCam cam(cam_id);

    TeliCam::Parameters cam_params;
    cam_params.exposure = 20000.0f;
    cam_params.framerate = 40.0f;

    cam.initialize(cam_params);
    cam.start_stream();

    // Display TeliCam video feed ///////////////////////////////////
    cv::namedWindow("Telicam", cv::WINDOW_AUTOSIZE);

    char key = 0;
    while (key != 27)
    {
        cv::imshow("Telicam", cam.get_last_frame());
        key = cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1 / cam_params.framerate)));
    }

    // Close the TeliCam ////////////////////////////////////////////
    cam.destroy();
    TeliCam::close_api();

    return 0;
}
```

## TeliCam Viewer Usage
The TeliCam Viewer application can be launched as such:

`./telicam_viewer --config params.json`

It takes as input a JSON file containing the parameters for the camera. Below is an example:
```json
{
    "width": 0,
    "height": 0,
    "offset_x": 0,
    "offset_y": 0,
    "binning_x": 0,
    "binning_y": 0,
    "decimation_x": 0,
    "decimation_y": 0,
    "exposure_time": 25000.0,
    "saturation": 100.0,
    "gamma": 1.0,
    "hue": 0.0,
    "gain": 0.0,
    "auto_gain": false,
    "black_level": 0.0,
    "framerate": 30.0,
    "sharpness": 0,
    "balance_ratio_r": 4.0,
    "balance_ratio_b": 4.0,
    "auto_white_balance": true,
    "reverse_x": false,
    "reverse_y": false,
    "trigger_mode": false
}
```