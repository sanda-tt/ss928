#include "vot_pipeline.h"

#include <cassert>
#include <iostream>

int main() {
    VotPipelineConfig config;
    config.image_width = 640;
    config.image_height = 480;
    config.distance_mode = "size";
    config.min_warning_frames = 1;
    config.distance_smoothing = 1.0;
    VotPipeline pipeline(config);

    std::vector<VotPipelineTarget> last;
    for (int frame = 0; frame < 12; ++frame) {
        BackendDetection detection;
        const double height = 40.0 + frame * 22.0;
        detection.x1 = 320.0 - height * 0.9;
        detection.x2 = 320.0 + height * 0.9;
        detection.y2 = 440.0;
        detection.y1 = detection.y2 - height;
        detection.class_id = 2;
        detection.score = 0.9f;
        last = pipeline.update({detection}, frame * 0.1);
        assert(last.size() == 1);
        assert(last[0].tracked.track_id == 1);
    }
    assert(last[0].tracked.has_distance);
    assert(last[0].tracked.speed_mps >= 0.0);

    VotPipelineTarget event = last[0];
    event.risk.level = VOT_RISK_DANGER;
    event.risk.score = 0.7421;
    event.risk.has_ttc = true;
    event.risk.ttc_s = 1.4;
    event.risk.has_trajectory_distance = true;
    event.risk.trajectory_distance_m = 0.8;
    const std::string json = vision_alert_json("left", event);
    assert(json.find("\"type\":\"vision_alert\"") != std::string::npos);
    assert(json.find("\"side\":\"left\"") != std::string::npos);
    assert(json.find("\"level\":3") != std::string::npos);
    assert(json.find("\"track_id\":1") != std::string::npos);
    assert(json.find("\n") == std::string::npos);
    std::cout << json << '\n';
    std::cout << "VOT pipeline tests passed\n";
    return 0;
}
