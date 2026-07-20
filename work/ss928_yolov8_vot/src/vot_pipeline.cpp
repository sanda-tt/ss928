#include "vot_pipeline.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>

namespace {

VotDetectionObservation detection_to_observation(
    const BackendDetection &detection,
    const VotCameraCalibration &calibration,
    const VotPipelineConfig &config,
    double timestamp_s) {
    VotDetectionObservation observation{};
    observation.track_id = detection.track_id;
    vot_copy_class_name(observation.class_name, sizeof(observation.class_name),
                        backend_coco_class_name(detection.class_id));
    observation.confidence = detection.score;
    observation.bbox = {detection.x1, detection.y1, detection.x2, detection.y2};
    observation.timestamp_s = timestamp_s;
    VotDistanceEstimate estimate{};
    if (vot_estimate_ground_point_from_bbox(observation.bbox, observation.class_name,
            &calibration, config.distance_mode.c_str(), config.size_weight, &estimate)) {
        observation.has_ground_point = true;
        observation.ground_point = estimate.point;
        vot_copy_class_name(observation.distance_source, sizeof(observation.distance_source), estimate.source);
    } else {
        vot_copy_class_name(observation.distance_source, sizeof(observation.distance_source), "unknown");
    }
    return observation;
}

void append_number_or_null(std::ostringstream &stream, bool available, double value) {
    if (available && std::isfinite(value)) stream << value;
    else stream << "null";
}

}  // namespace

VotPipeline::VotPipeline(const VotPipelineConfig &config) : config_(config) {
    calibration_ = vot_camera_calibration_default();
    calibration_.image_width = config.image_width;
    calibration_.image_height = config.image_height;
    calibration_.camera_height_m = config.camera_height;
    calibration_.camera_pitch_deg = config.camera_pitch;
    calibration_.fov_deg = config.fov;
    calibration_.has_horizontal_fov = config.has_horizontal_fov;
    calibration_.horizontal_fov_deg = config.horizontal_fov;
    calibration_.distance_scale = config.distance_scale;
    vot_copy_class_name(calibration_.fov_type, sizeof(calibration_.fov_type), config.fov_type.c_str());
    vot_stable_track_id_manager_init(&stable_ids_, 2.0, 1.0);
    vot_track_state_init(&track_state_, config.speed_window, config.distance_smoothing,
                         config.max_speed, config.speed_scale);
    risk_config_ = vot_risk_model_config_default();
    vot_risk_warning_stabilizer_init(&risk_stabilizer_, config.min_warning_frames);
}

std::vector<VotPipelineTarget> VotPipeline::update(
    const std::vector<BackendDetection> &detections, double timestamp_s) {
    const std::vector<BackendDetection> tracked_detections = raw_tracker_.update(detections);
    VotDetectionObservation raw[VOT_MAX_OBSERVATIONS]{};
    const std::size_t raw_count = std::min(tracked_detections.size(), static_cast<std::size_t>(VOT_MAX_OBSERVATIONS));
    for (std::size_t i = 0; i < raw_count; ++i) {
        raw[i] = detection_to_observation(tracked_detections[i], calibration_, config_, timestamp_s);
    }
    VotDetectionObservation stable[VOT_MAX_OBSERVATIONS]{};
    const std::size_t stable_count = vot_stable_track_assign(
        &stable_ids_, raw, raw_count, stable, VOT_MAX_OBSERVATIONS);
    std::vector<VotPipelineTarget> result;
    result.reserve(stable_count);
    for (std::size_t i = 0; i < stable_count; ++i) {
        VotPipelineTarget target;
        target.tracked = vot_track_state_update(&track_state_, stable[i]);
        target.risk = vot_risk_warning_stabilize_one(
            &risk_stabilizer_, vot_assess_collision_risk(target.tracked, &risk_config_));
        result.push_back(target);
    }
    return result;
}

const VotPipelineTarget *VotPipeline::highest_non_safe(
    const std::vector<VotPipelineTarget> &targets) const {
    const VotPipelineTarget *best = nullptr;
    for (const VotPipelineTarget &candidate : targets) {
        if (candidate.risk.level == VOT_RISK_SAFE) continue;
        if (best == nullptr || candidate.risk.level > best->risk.level ||
            (candidate.risk.level == best->risk.level && candidate.risk.score > best->risk.score) ||
            (candidate.risk.level == best->risk.level && candidate.risk.score == best->risk.score &&
             candidate.risk.has_ttc &&
             (!best->risk.has_ttc || candidate.risk.ttc_s < best->risk.ttc_s))) {
            best = &candidate;
        }
    }
    return best;
}

std::string vision_alert_json(const std::string &side, const VotPipelineTarget &target) {
    std::ostringstream stream;
    stream << std::fixed << std::setprecision(4)
           << "{\"type\":\"vision_alert\",\"side\":\"" << (side == "right" ? "right" : "left")
           << "\",\"level\":" << static_cast<int>(target.risk.level)
           << ",\"score\":" << target.risk.score
           << ",\"track_id\":" << target.tracked.track_id
           << ",\"ts\":" << target.tracked.timestamp_s
           << ",\"class_name\":\"" << target.tracked.class_name
           << "\",\"confidence\":" << target.tracked.confidence
           << ",\"distance_m\":";
    append_number_or_null(stream, target.tracked.has_distance, target.tracked.distance_m);
    stream << ",\"speed_mps\":";
    append_number_or_null(stream, target.tracked.has_ground_point, target.tracked.speed_mps);
    stream << ",\"vx_mps\":";
    append_number_or_null(stream, target.tracked.has_ground_point, target.tracked.vx_mps);
    stream << ",\"vz_mps\":";
    append_number_or_null(stream, target.tracked.has_ground_point, target.tracked.vz_mps);
    stream << ",\"ttc_s\":";
    append_number_or_null(stream, target.risk.has_ttc, target.risk.ttc_s);
    stream << ",\"trajectory_distance_m\":";
    append_number_or_null(stream, target.risk.has_trajectory_distance, target.risk.trajectory_distance_m);
    stream << '}';
    return stream.str();
}
