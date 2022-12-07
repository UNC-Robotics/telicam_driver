#include <fstream>
#include <iostream>
#include <random>
#include <sstream>
#include <thread>
#include <filesystem>

#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
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

    // Check if ./data exists, and if not create it
    std::filesystem::path data_path("./data");
    if (!std::filesystem::exists(data_path))
    {
        std::filesystem::create_directory(data_path);
    }

    /////////////////////////////////////////////
    // Run TeliCam
    /////////////////////////////////////////////
    TeliCam::initialize_api();

    TeliCam cam0(0);
    TeliCam cam1(1);
    TeliCam::Parameters cam_params = read_json(config_filename);

    cam0.initialize(cam_params);
    cam1.initialize(cam_params);

    if (!capture_mode)
    {
        cam0.start_stream();
        cam1.start_stream();
    }

    cv::namedWindow("Telicam", cv::WINDOW_NORMAL);
    cam0.print_system_info();
    cam0.print_camera_info();
    cam0.print_parameters();

    char key = 0;
    while (key != 27)
    {
        cv::Mat frame0 = cam0.get_last_frame();
        cv::Mat frame1 = cam1.get_last_frame();
        // Resize to 720p
        cv::resize(frame0, frame0, cv::Size(1024, 768));
        cv::resize(frame1, frame1, cv::Size(1024, 768));

        // Dislay side by side
        cv::Mat frame;
        cv::hconcat(frame0, frame1, frame);
        cv::imshow("Telicam", frame);
        key = cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1 / cam_params.framerate)));

        // If key equals spacebar
        if (capture_mode && key == 32)
        {
            cam0.capture_frame();
            cam1.capture_frame();
        }

        // If key equals g
        if (key == 103)
        {
            // Write the last frame to disk as a JPG
            // Generate GUID
            std::string guid = uuid::generate_uuid_v4();
            std::stringstream filename;
            filename << "./data/" << guid << ".jpg";
            cv::imwrite(filename.str(), cam0.get_last_frame());
        }
    }

    cam0.destroy();
    cam1.destroy();

    TeliCam::close_api();

    return 0;
}
