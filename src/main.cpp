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
    cam.start();

    cv::namedWindow("Telicam", cv::WINDOW_AUTOSIZE);

    while (1)
    {
        cv::imshow("Telicam", cam.get_last_frame());
        cv::waitKey(1);
        //std::this_thread::sleep_for(std::chrono::milliseconds(25));
    }

    return 0;
}
