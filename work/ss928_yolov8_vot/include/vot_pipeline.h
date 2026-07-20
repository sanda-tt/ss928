#ifndef VOT_PIPELINE_H
#define VOT_PIPELINE_H

#include "vot.h"
#include "vot_backend.h"

#include <string>
#include <vector>

struct VotPipelineConfig {
    int image_width = 1920;
    int image_height = 1080;
    double camera_height = 1.2;
    double camera_pitch = 5.0;
    double fov = 120.0;
    std::string fov_type = "diagonal";
    bool has_horizontal_fov = false;
    double horizontal_fov = 0.0;
    std::string distance_mode = "fused";
    double size_weight = 0.75;
    double distance_scale = 1.0;
    double speed_scale = 1.0;
    double speed_window = 1.5;
    double distance_smoothing = 0.35;
    double max_speed = 40.0;
    int min_warning_frames = 3;
};

struct VotPipelineTarget {
    VotTrackedObject tracked{};
    VotRiskAssessment risk{};
};

class VotPipeline {
public:
    explicit VotPipeline(const VotPipelineConfig &config);
    std::vector<VotPipelineTarget> update(const std::vector<BackendDetection> &detections, double timestamp_s);
    const VotPipelineTarget *highest_non_safe(const std::vector<VotPipelineTarget> &targets) const;

private:
    VotPipelineConfig config_;
    VotCameraCalibration calibration_{};
    BackendSimpleTracker raw_tracker_;
    VotStableTrackIdManager stable_ids_{};
    VotTrackState track_state_{};
    VotRiskModelConfig risk_config_{};
    VotRiskWarningStabilizer risk_stabilizer_{};
};

std::string vision_alert_json(const std::string &side, const VotPipelineTarget &target);

#endif
