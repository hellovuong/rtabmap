#pragma once
#include "nlohmann/json.hpp"
#include <opencv2/opencv.hpp>

namespace rtabmap {
// Convert cv::KeyPoint to JSON
static nlohmann::json keypoint_to_json(const cv::KeyPoint &keypoint) {
  return {
      {"x", keypoint.pt.x},
      {"y", keypoint.pt.y},
  };
}

// Convert std::vector<cv::KeyPoint> to JSON
[[maybe_unused]] static nlohmann::json
keypoints_to_json(const std::vector<cv::KeyPoint> &keypoints) {
  nlohmann::json json_keypoints;
  for (const auto &keypoint : keypoints) {
    json_keypoints.push_back(keypoint_to_json(keypoint));
  }
  return json_keypoints;
}

// Convert cv::Mat descriptor to JSON
[[maybe_unused]] static nlohmann::json
descriptor_to_json(const cv::Mat &descriptors) {
  if (descriptors.empty()) {
    return nullptr;
  }
  // Convert cv::Mat to a 2D std::vector
  std::vector<std::vector<float>> json_descriptors;
  json_descriptors.reserve(descriptors.rows);
  for (int i = 0; i < descriptors.rows; ++i) {
    json_descriptors.emplace_back(descriptors.ptr<float>(i),
                                  descriptors.ptr<float>(i) + descriptors.cols);
  }
  return json_descriptors;
}
} // namespace rtabmap
