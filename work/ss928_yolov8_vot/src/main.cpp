#include "frame_preprocess.h"
#include "model_tensor_info.h"
#include "ss928_acl_detector.h"
#include "vot_backend.h"
#include "vot_pipeline.h"
#include "yolov8_postprocess.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <csignal>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

volatile std::sig_atomic_t g_stop_requested = 0;

void signal_handler(int) { g_stop_requested = 1; }

struct Options {
    std::string model = "/opt/sample/yolov8/yolov8n.om";
    std::string source = "raw";
    std::string input;
    std::string camera = "/dev/video0";
    std::string side = "left";
    bool emit_alert_jsonl = false;
    bool no_display = false;
    bool print_model_info = false;
    std::string dump_output;
    int max_frames = 0;
    int repeat = 1;
    int source_width = 1920;
    int source_height = 1080;
    double source_fps = 30.0;
    float conf = 0.25f;
    float nms = 0.45f;
    int max_det = 50;
    std::string target_classes = "car,bicycle,motorcycle,bus,truck";
    VotPipelineConfig pipeline;
};

void usage(const char *program) {
    std::cerr
        << "Usage: " << program << " --model MODEL --source raw|image|video|camera [options]\n"
        << "  --input PATH --camera PATH --side left|right --emit-alert-jsonl --no-display\n"
        << "  --max-frames N --repeat N --conf F --nms F --max-det N --target-classes LIST|all\n"
        << "  --source-width N --source-height N --source-fps F --print-model-info --dump-output PATH\n"
        << "  --camera-height F --camera-pitch F --fov F --fov-type diagonal|horizontal|vertical\n"
        << "  --horizontal-fov F --distance-mode ground|size|fused --size-weight F\n"
        << "  --distance-scale F --speed-scale F --speed-window F --distance-smoothing F --max-speed F\n";
}

bool require_value(int argc, char **argv, int *index, std::string *value, std::string *error) {
    if (*index + 1 >= argc) {
        *error = std::string("missing value for ") + argv[*index];
        return false;
    }
    *value = argv[++(*index)];
    return true;
}

bool parse_int(const std::string &text, int *value) {
    char *end = nullptr;
    const long parsed = std::strtol(text.c_str(), &end, 10);
    if (end == text.c_str() || *end != '\0' || parsed < std::numeric_limits<int>::min() ||
        parsed > std::numeric_limits<int>::max()) return false;
    *value = static_cast<int>(parsed);
    return true;
}

bool parse_double(const std::string &text, double *value) {
    char *end = nullptr;
    const double parsed = std::strtod(text.c_str(), &end);
    if (end == text.c_str() || *end != '\0' || !std::isfinite(parsed)) return false;
    *value = parsed;
    return true;
}

bool parse_options(int argc, char **argv, Options *options, std::string *error) {
    for (int i = 1; i < argc; ++i) {
        const std::string key = argv[i];
        if (key == "--help" || key == "-h") {
            usage(argv[0]);
            std::exit(0);
        }
        if (key == "--emit-alert-jsonl") { options->emit_alert_jsonl = true; continue; }
        if (key == "--no-display") { options->no_display = true; continue; }
        if (key == "--print-model-info") { options->print_model_info = true; continue; }
        std::string value;
        if (!require_value(argc, argv, &i, &value, error)) return false;
        if (key == "--model") options->model = value;
        else if (key == "--source") options->source = value;
        else if (key == "--input") options->input = value;
        else if (key == "--camera") options->camera = value;
        else if (key == "--side") options->side = value;
        else if (key == "--target-classes") options->target_classes = value;
        else if (key == "--dump-output") options->dump_output = value;
        else if (key == "--fov-type") options->pipeline.fov_type = value;
        else if (key == "--distance-mode") options->pipeline.distance_mode = value;
        else if (key == "--max-frames") { if (!parse_int(value, &options->max_frames)) goto bad_value; }
        else if (key == "--repeat") { if (!parse_int(value, &options->repeat)) goto bad_value; }
        else if (key == "--source-width") { if (!parse_int(value, &options->source_width)) goto bad_value; }
        else if (key == "--source-height") { if (!parse_int(value, &options->source_height)) goto bad_value; }
        else if (key == "--max-det") { if (!parse_int(value, &options->max_det)) goto bad_value; }
        else if (key == "--source-fps") { if (!parse_double(value, &options->source_fps)) goto bad_value; }
        else if (key == "--conf") { double v; if (!parse_double(value, &v)) goto bad_value; options->conf = static_cast<float>(v); }
        else if (key == "--nms") { double v; if (!parse_double(value, &v)) goto bad_value; options->nms = static_cast<float>(v); }
        else if (key == "--camera-height") { if (!parse_double(value, &options->pipeline.camera_height)) goto bad_value; }
        else if (key == "--camera-pitch") { if (!parse_double(value, &options->pipeline.camera_pitch)) goto bad_value; }
        else if (key == "--fov") { if (!parse_double(value, &options->pipeline.fov)) goto bad_value; }
        else if (key == "--horizontal-fov") {
            if (!parse_double(value, &options->pipeline.horizontal_fov)) goto bad_value;
            options->pipeline.has_horizontal_fov = true;
        }
        else if (key == "--size-weight") { if (!parse_double(value, &options->pipeline.size_weight)) goto bad_value; }
        else if (key == "--distance-scale") { if (!parse_double(value, &options->pipeline.distance_scale)) goto bad_value; }
        else if (key == "--speed-scale") { if (!parse_double(value, &options->pipeline.speed_scale)) goto bad_value; }
        else if (key == "--speed-window") { if (!parse_double(value, &options->pipeline.speed_window)) goto bad_value; }
        else if (key == "--distance-smoothing") { if (!parse_double(value, &options->pipeline.distance_smoothing)) goto bad_value; }
        else if (key == "--max-speed") { if (!parse_double(value, &options->pipeline.max_speed)) goto bad_value; }
        else { *error = "unknown option: " + key; return false; }
        continue;
bad_value:
        *error = "invalid value for " + key + ": " + value;
        return false;
    }
    if (options->side != "left" && options->side != "right") {
        *error = "--side must be left or right";
        return false;
    }
    if (options->source != "raw" && options->source != "image" &&
        options->source != "video" && options->source != "camera") {
        *error = "--source must be raw, image, video, or camera";
        return false;
    }
    if (options->repeat <= 0 || options->max_det <= 0 || options->source_width <= 0 ||
        options->source_height <= 0 || options->source_fps <= 0.0 || options->conf < 0.0f ||
        options->conf > 1.0f || options->nms < 0.0f || options->nms > 1.0f) {
        *error = "numeric option out of range";
        return false;
    }
    options->pipeline.image_width = options->source_width;
    options->pipeline.image_height = options->source_height;
    return true;
}

bool read_exact_file(const std::string &path, std::size_t expected,
                     std::vector<unsigned char> *bytes, std::string *error) {
    std::ifstream input(path, std::ios::binary | std::ios::ate);
    if (!input) { *error = "could not open input: " + path; return false; }
    const std::streamoff size = input.tellg();
    if (size < 0 || static_cast<std::size_t>(size) != expected) {
        std::ostringstream stream;
        stream << "input size mismatch: expected " << expected << " bytes, got " << size;
        *error = stream.str();
        return false;
    }
    bytes->resize(expected);
    input.seekg(0);
    input.read(reinterpret_cast<char *>(bytes->data()), static_cast<std::streamsize>(expected));
    if (!input) { *error = "could not read input: " + path; return false; }
    return true;
}

void print_model_info(const Ss928AclDetector &detector) {
    for (std::size_t i = 0; i < detector.inputs().size(); ++i)
        std::cerr << "input=" << tensor_json(detector.inputs()[i], i) << '\n';
    for (std::size_t i = 0; i < detector.outputs().size(); ++i)
        std::cerr << "output=" << tensor_json(detector.outputs()[i], i) << '\n';
}

struct FloatStats {
    double minimum = std::numeric_limits<double>::infinity();
    double maximum = -std::numeric_limits<double>::infinity();
    double mean = 0.0;
    std::size_t finite = 0;
    std::size_t nan = 0;
    std::size_t inf = 0;
};

FloatStats calculate_stats(const float *data, std::size_t count) {
    FloatStats stats;
    long double sum = 0.0;
    for (std::size_t i = 0; i < count; ++i) {
        const double value = data[i];
        if (std::isnan(value)) { ++stats.nan; continue; }
        if (!std::isfinite(value)) { ++stats.inf; continue; }
        stats.minimum = std::min(stats.minimum, value);
        stats.maximum = std::max(stats.maximum, value);
        sum += value;
        ++stats.finite;
    }
    if (stats.finite != 0) stats.mean = static_cast<double>(sum / stats.finite);
    return stats;
}

void print_layout_diagnostics(const float *data, std::size_t count) {
    if (count != 84u * 8400u) return;
    float channel_first_max = -std::numeric_limits<float>::infinity();
    int channel_first_class = -1;
    std::size_t channel_first_anchor = 0;
    float anchor_first_max = -std::numeric_limits<float>::infinity();
    int anchor_first_class = -1;
    std::size_t anchor_first_anchor = 0;
    for (int class_id = 0; class_id < 80; ++class_id) {
        for (std::size_t anchor = 0; anchor < 8400; ++anchor) {
            const float cf = data[static_cast<std::size_t>(class_id + 4) * 8400 + anchor];
            if (std::isfinite(cf) && cf > channel_first_max) {
                channel_first_max = cf;
                channel_first_class = class_id;
                channel_first_anchor = anchor;
            }
            const float af = data[anchor * 84 + static_cast<std::size_t>(class_id + 4)];
            if (std::isfinite(af) && af > anchor_first_max) {
                anchor_first_max = af;
                anchor_first_class = class_id;
                anchor_first_anchor = anchor;
            }
        }
    }
    std::cerr << "layout_probe channel_first_max=" << channel_first_max
              << " class=" << channel_first_class << " anchor=" << channel_first_anchor
              << " anchor_first_max=" << anchor_first_max
              << " class=" << anchor_first_class << " anchor=" << anchor_first_anchor << '\n';
}

}  // namespace

int main(int argc, char **argv) {
    Options options;
    std::string error;
    if (!parse_options(argc, argv, &options, &error)) {
        std::cerr << "argument error: " << error << '\n';
        usage(argv[0]);
        return 2;
    }
    if (options.source != "raw") {
        std::cerr << "source " << options.source << " is not enabled in the offline-stage binary yet\n";
        return 2;
    }
    if (options.input.empty()) {
        std::cerr << "argument error: --input is required for raw source\n";
        return 2;
    }

    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);
    Ss928AclDetector detector;
    if (!detector.initialize(options.model, 0, &error)) {
        std::cerr << error << '\n';
        return 1;
    }
    if (options.print_model_info) print_model_info(detector);

    std::vector<unsigned char> input;
    if (!read_exact_file(options.input, detector.input_bytes(), &input, &error)) {
        std::cerr << error << '\n';
        return 1;
    }
    LetterboxInfo letterbox;
    if (!compute_letterbox(options.source_width, options.source_height, 640, 640, &letterbox, &error)) {
        std::cerr << "letterbox error: " << error << '\n';
        return 1;
    }
    const BackendTargetClassFilter target_filter = backend_parse_target_classes(options.target_classes);
    VotPipeline pipeline(options.pipeline);
    int frames = options.repeat;
    if (options.max_frames > 0) frames = std::min(frames, options.max_frames);
    int successful = 0;
    double inference_total_ms = 0.0;
    double post_total_ms = 0.0;
    const auto total_begin = std::chrono::steady_clock::now();
    std::cerr << "input mode=raw path=" << options.input << " bytes=" << input.size()
              << " source=" << options.source_width << 'x' << options.source_height
              << " model=640x640 format=NV12\n";

    for (int frame = 0; frame < frames && !g_stop_requested; ++frame) {
        AclInferenceResult inference;
        if (!detector.infer(input.data(), input.size(), &inference, &error)) {
            std::cerr << error << '\n';
            return 1;
        }
        const auto post_begin = std::chrono::steady_clock::now();
        std::vector<BackendDetection> detections;
        YoloV8DecodeStats decode_stats;
        if (!decode_yolov8(inference.output, inference.output_elements, inference.output_dims,
                letterbox, target_filter, options.conf, options.nms, options.max_det,
                &detections, &decode_stats, &error)) {
            std::cerr << "decode error: " << error << '\n';
            return 1;
        }
        const double timestamp_s = static_cast<double>(frame) / options.source_fps;
        const std::vector<VotPipelineTarget> targets = pipeline.update(detections, timestamp_s);
        const auto post_end = std::chrono::steady_clock::now();
        const double post_ms = std::chrono::duration<double, std::milli>(post_end - post_begin).count();
        inference_total_ms += inference.inference_ms;
        post_total_ms += post_ms;
        ++successful;

        if (frame == 0) {
            if (!options.dump_output.empty()) {
                std::ofstream dump(options.dump_output, std::ios::binary | std::ios::trunc);
                dump.write(reinterpret_cast<const char *>(inference.output),
                           static_cast<std::streamsize>(inference.output_elements * sizeof(float)));
                if (!dump) {
                    std::cerr << "could not write output dump: " << options.dump_output << '\n';
                    return 1;
                }
            }
            const FloatStats stats = calculate_stats(inference.output, inference.output_elements);
            std::cerr << std::fixed << std::setprecision(6)
                      << "output_stats elements=" << inference.output_elements
                      << " min=" << stats.minimum << " max=" << stats.maximum << " mean=" << stats.mean
                      << " nan=" << stats.nan << " inf=" << stats.inf << '\n';
            print_layout_diagnostics(inference.output, inference.output_elements);
            for (const BackendDetection &detection : detections) {
                std::cerr << std::fixed << std::setprecision(3)
                          << "detection class_id=" << detection.class_id
                          << " class_name=" << backend_coco_class_name(detection.class_id)
                          << " score=" << detection.score << " bbox=" << detection.x1 << ',' << detection.y1
                          << ',' << detection.x2 << ',' << detection.y2 << '\n';
            }
        }
        std::cerr << std::fixed << std::setprecision(3)
                  << "frame=" << frame << " infer_ms=" << inference.inference_ms << " post_ms=" << post_ms
                  << " raw_candidates=" << decode_stats.raw_candidate_count
                  << " nms_detections=" << decode_stats.nms_detection_count
                  << " tracked=" << targets.size() << '\n';
        for (const VotPipelineTarget &target : targets) {
            std::cerr << "target track_id=" << target.tracked.track_id
                      << " class_name=" << target.tracked.class_name
                      << " confidence=" << target.tracked.confidence
                      << " distance_m=" << (target.tracked.has_distance ? target.tracked.distance_m : -1.0)
                      << " speed_mps=" << target.tracked.speed_mps
                      << " vx_mps=" << target.tracked.vx_mps
                      << " vz_mps=" << target.tracked.vz_mps
                      << " risk=" << vot_risk_level_name(target.risk.level)
                      << " risk_score=" << target.risk.score << '\n';
        }
        const VotPipelineTarget *alert = pipeline.highest_non_safe(targets);
        if (options.emit_alert_jsonl && alert != nullptr) {
            std::cout << vision_alert_json(options.side, *alert) << std::endl;
        }
    }

    const auto total_end = std::chrono::steady_clock::now();
    const double total_ms = std::chrono::duration<double, std::milli>(total_end - total_begin).count();
    std::cerr << std::fixed << std::setprecision(3)
              << "summary requested=" << frames << " successful=" << successful
              << " model_loads=1 avg_infer_ms=" << (successful == 0 ? 0.0 : inference_total_ms / successful)
              << " avg_post_ms=" << (successful == 0 ? 0.0 : post_total_ms / successful)
              << " end_to_end_fps=" << (total_ms <= 0.0 ? 0.0 : successful * 1000.0 / total_ms)
              << " stopped=" << (g_stop_requested ? 1 : 0) << '\n';
    return successful == frames ? 0 : 130;
}
