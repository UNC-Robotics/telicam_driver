#include <iostream>
#include <thread>

#include <opencv2/highgui/highgui.hpp>

#include "telicam.hpp"

using namespace std;

int main(int argc, char **argv)
{
    std::cout << "Telicam driver" << std::endl;

    TeliCam::initialize_api();

    TeliCam cam(0);
    TeliCam::Parameters cam_params;

    cam.initialize(cam_params);
    cam.print_system_info();
    cam.print_camera_info();
    cam.start_stream();

    cv::namedWindow("Telicam", cv::WINDOW_AUTOSIZE);

    char key = 0;
    while (key != 27)
    {
        cv::imshow("Telicam", cam.get_last_frame());
        key = cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));

        // // If key equals spacebar
        // if (key == 32)
        // {
        //     cam.capture_frame();
        // }
    }

    cam.destroy();

    TeliCam::close_api();

    return 0;
}
