#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <CLI/CLI.hpp>
#include <nlohmann/json.hpp>

#include "telicam.hpp"
#include "uuid.hpp"

using namespace std;
using json = nlohmann::json;

struct ViewerTeliCamParams
{
    int cam_id;
    TeliCam::Parameters camera_params;
    int downscale_factor;
};

std::vector<ViewerTeliCamParams> read_config(std::string filename)
{
    std::ifstream file(filename);
    json data = json::parse(file);

    std::vector<ViewerTeliCamParams> all_params;
    for (auto& cam : data["cameras"])
    {
        const json& params_json = cam["params"];

        ViewerTeliCamParams params;
        params.cam_id = cam["cam_id"].get<int>();
        params.camera_params.width = params_json["width"].get<uint32_t>();
        params.camera_params.height = params_json["height"].get<uint32_t>();
        params.camera_params.offset_x = params_json["offset_x"].get<uint32_t>();
        params.camera_params.offset_y = params_json["offset_y"].get<uint32_t>();
        params.camera_params.binning_x = params_json["binning_x"].get<uint32_t>();
        params.camera_params.binning_y = params_json["binning_y"].get<uint32_t>();
        params.camera_params.decimation_x = params_json["decimation_x"].get<uint32_t>();
        params.camera_params.decimation_y = params_json["decimation_y"].get<uint32_t>();
        params.camera_params.exposure_time = params_json["exposure_time"].get<float64_t>();
        params.camera_params.saturation = params_json["saturation"].get<float64_t>();
        params.camera_params.gamma = params_json["gamma"].get<float64_t>();
        params.camera_params.hue = params_json["hue"].get<float64_t>();
        params.camera_params.gain = params_json["gain"].get<float64_t>();
        params.camera_params.auto_gain = params_json["auto_gain"].get<bool>();
        params.camera_params.black_level = params_json["black_level"].get<float64_t>();
        params.camera_params.framerate = params_json["framerate"].get<float64_t>();
        params.camera_params.sharpness = params_json["sharpness"].get<uint32_t>();
        params.camera_params.balance_ratio_r = params_json["balance_ratio_r"].get<float64_t>();
        params.camera_params.balance_ratio_b = params_json["balance_ratio_b"].get<float64_t>();
        params.camera_params.auto_white_balance = params_json["auto_white_balance"].get<bool>();
        params.camera_params.reverse_x = params_json["reverse_x"].get<bool>();
        params.camera_params.reverse_y = params_json["reverse_y"].get<bool>();
        params.camera_params.trigger_mode = params_json["trigger_mode"].get<bool>();

        params.downscale_factor = cam["downscale_factor"].get<int>();

        all_params.push_back(params);
    }

    return all_params;
}

int main(int argc, char** argv)
{
    /////////////////////////////////////////////
    // Parse CLI arguments
    /////////////////////////////////////////////
    CLI::App app{"TeliCam viewer"};

    std::string config_filename;
    std::vector<int> cam_ids;
    bool capture_mode = false;
    int refresh_rate = 30;

    app.add_option("--cam", cam_ids, "List of camera IDs to ppen")->required();
    app.add_option("--config", config_filename, "Configuration file")->required()->check(CLI::ExistingFile);
    app.add_flag("--capture", capture_mode, "Capture mode")->default_val(false);
    app.add_option("--refresh", refresh_rate, "Refresh rate (Hz)")->default_val(30);

    CLI11_PARSE(app, argc, argv);

    // Check if ./data exists, and if not create it
    std::filesystem::path data_path("./data");
    if (!std::filesystem::exists(data_path))
    {
        std::filesystem::create_directory(data_path);
    }

    /////////////////////////////////////////////
    // Initialize TeliCams
    /////////////////////////////////////////////
    TeliCam::initialize_api();

    // Create cameras
    std::vector<TeliCam> cams;
    for (auto id : cam_ids)
    {
        cams.push_back(TeliCam(id));
    }

    // Initialize cameras and start streams
    std::vector<ViewerTeliCamParams> params = read_config(config_filename);
    for (int i = 0; i < cams.size(); ++i)
    {
        cams[i].initialize(params[i].camera_params);
        if (!capture_mode)
        {
            cams[i].start_stream();
        }
    }

    // Print camera info
    cams[0].print_system_info();
    for (auto& cam : cams)
    {
        cam.print_camera_info();
    }

    /////////////////////////////////////////////
    // Display TeliCam Streams
    /////////////////////////////////////////////
    cv::namedWindow("TeliCam", cv::WINDOW_NORMAL);

    std::vector<cv::Mat> cam_frames(cams.size());
    char key = 0;
    while (key != 27)
    {
        for (int i = 0; i < cams.size(); ++i)
        {
            cam_frames[i] = cams[i].get_last_frame();

            // Downscale frame by downscale_factor
            int new_width = cam_frames[i].cols / params[i].downscale_factor;
            int new_height = cam_frames[i].rows / params[i].downscale_factor;

            cv::resize(cam_frames[i], cam_frames[i], cv::Size(new_width, new_height));
        }

        // Dislay side by side
        cv::Mat frame;
        cv::hconcat(cam_frames, frame);
        cv::imshow("TeliCam", frame);

        key = cv::waitKey(1);
        std::this_thread::sleep_for(std::chrono::milliseconds((int)(1 / refresh_rate)));

        // If key equals spacebar
        if (capture_mode && key == 32)
        {
            for (auto& cam : cams)
            {
                cam.capture_frame();
            }
        }

        // If key equals g, write last captured frame to the disk
        if (key == 103)
        {
            for (auto& cam : cams)
            {
                std::string guid = uuid::generate_uuid_v4();
                std::stringstream filename;
                filename << "./data/" << guid << ".jpg";
                cv::imwrite(filename.str(), cam.get_last_frame());
            }
        }
    }

    // Destroy cameras
    for (auto& cam : cams)
    {
        cam.destroy();
    }

    TeliCam::close_api();

    return 0;
}
