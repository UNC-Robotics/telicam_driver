#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>

#include <opencv2/highgui/highgui.hpp>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include "telicam.hpp"

using namespace std;
using json = nlohmann::json;

TeliCam::Parameters read_json(std::string filename)
{
    std::ifstream file(filename);
    json data = json::parse(file);

    TeliCam::Parameters params;
    params.width = data["width"].get<uint32_t>();
    params.height = data["height"].get<uint32_t>();
    params.offset_x = data["offset_x"].get<uint32_t>();
    params.offset_y = data["offset_y"].get<uint32_t>();
    params.binning_x = data["binning_x"].get<uint32_t>();
    params.binning_y = data["binning_y"].get<uint32_t>();
    params.decimation_x = data["decimation_x"].get<uint32_t>();
    params.decimation_y = data["decimation_y"].get<uint32_t>();
    params.exposure_time = data["exposure_time"].get<float64_t>();
    params.saturation = data["saturation"].get<float64_t>();
    params.gamma = data["gamma"].get<float64_t>();
    params.hue = data["hue"].get<float64_t>();
    params.gain = data["gain"].get<float64_t>();
    params.auto_gain = data["auto_gain"].get<bool>();
    params.black_level = data["black_level"].get<float64_t>();
    params.framerate = data["framerate"].get<float64_t>();
    params.sharpness = data["sharpness"].get<uint32_t>();
    params.balance_ratio_r = data["balance_ratio_r"].get<float64_t>();
    params.balance_ratio_b = data["balance_ratio_b"].get<float64_t>();
    params.auto_white_balance = data["auto_white_balance"].get<bool>();
    params.reverse_x = data["reverse_x"].get<bool>();
    params.reverse_y = data["reverse_y"].get<bool>();
    params.trigger_mode = data["trigger_mode"].get<bool>();

    return params;
}

namespace uuid
{
static std::random_device rd;
static std::mt19937 gen(rd());
static std::uniform_int_distribution<> dis(0, 15);
static std::uniform_int_distribution<> dis2(8, 11);

std::string generate_uuid_v4()
{
    std::stringstream ss;
    int i;
    ss << std::hex;
    for (i = 0; i < 8; i++)
    {
        ss << dis(gen);
    }
    for (i = 0; i < 4; i++)
    {
        ss << dis(gen);
    }
    for (i = 0; i < 3; i++)
    {
        ss << dis(gen);
    }
    ss << dis2(gen);
    for (i = 0; i < 3; i++)
    {
        ss << dis(gen);
    }
    for (i = 0; i < 12; i++)
    {
        ss << dis(gen);
    };
    return ss.str();
}
} // namespace uuid

int main(int argc, char **argv)
{
    /////////////////////////////////////////////
    // Parse CLI arguments
    /////////////////////////////////////////////
    CLI::App app{"TeliCam viewer"};

    std::string config_filename;
    int cam_id = 0;
    bool capture_mode = false;
    app.add_option("--cam_id", cam_id, "Camera ID")->default_val(0);
    app.add_option("--config", config_filename, "Configuration file")->required()->check(CLI::ExistingFile);
    app.add_flag("--capture", capture_mode, "Capture mode");

    CLI11_PARSE(app, argc, argv);

    /////////////////////////////////////////////
    // Run TeliCam
    /////////////////////////////////////////////
    TeliCam::initialize_api();

    TeliCam cam(cam_id);
    TeliCam::Parameters cam_params = read_json(config_filename);

    cam.initialize(cam_params);
    cam.print_system_info();
    cam.print_camera_info();

    if (!capture_mode)
    {
        cam.start_stream();
    }

    cv::namedWindow("Telicam", cv::WINDOW_AUTOSIZE);

    char key = 0;
    while (key != 27)
    {
        cv::imshow("Telicam", cam.get_last_frame());
        key = cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1 / cam_params.framerate)));

        // If key equals spacebar
        if (capture_mode && key == 32)
        {
            cam.capture_frame();
        }

        // If key equals g
        if (key == 103)
        {
            // Write the last frame to disk as a JPG
            // Generate GUID
            std::string guid = uuid::generate_uuid_v4();
            std::string filename = guid + ".jpg";
            cv::imwrite(filename, cam.get_last_frame());
        }
    }

    cam.destroy();

    TeliCam::close_api();

    return 0;
}
