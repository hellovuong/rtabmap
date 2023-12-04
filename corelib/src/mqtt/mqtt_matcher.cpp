#include "nlohmann/json.hpp"
#include <chrono>
#include <rtabmap/core/mqtt/mqtt_matcher.h>
#include <string>
#include <thread>

namespace rtabmap {
MqttMatcher::MqttMatcher(std::string_view server_addrs,
                         std::string_view input_topic,
                         std::string_view output_topic)
    : client_(server_addrs.data(), ""), input_topic_(input_topic) {

  client_.set_callback(*this);
  // Connect to the MQTT broker
  mqtt::connect_options conn_opts;
  conn_opts.set_keep_alive_interval(20);
  conn_opts.set_clean_session(true);

  try {
    client_.connect(conn_opts)->wait();
  } catch (const mqtt::exception &exc) {
    std::cerr << "Error connecting to MQTT broker: " << exc.what() << std::endl;
  }

  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
            << " Connected to server_addrs: " << server_addrs << std::endl;
  // Subscribe to the result topic
  client_.subscribe(output_topic.data(), 1);
  UINFO("Started mqtt_extractor!");
}

void MqttMatcher::request_matching(const std::vector<cv::KeyPoint> &from_kps,
                                   const cv::Mat &from_desc,
                                   const std::vector<cv::KeyPoint> &to_kps,
                                   const cv::Mat &to_desc,
                                   std::vector<cv::DMatch> &matches) {
  matches_.clear();
  // Convert to JSON
  nlohmann::json json_from_keypoints = keypoints_to_json(from_kps);
  nlohmann::json json_from_descriptor = descriptor_to_json(from_desc);

  nlohmann::json json_to_keypoints = keypoints_to_json(to_kps);
  nlohmann::json json_to_descriptor = descriptor_to_json(to_desc);

  // Combine into a single JSON object
  nlohmann::json combined_json;
  combined_json["keypoints_0"] = json_from_keypoints;
  combined_json["descriptor_0"] = json_from_descriptor;
  combined_json["keypoints_1"] = json_to_keypoints;
  combined_json["descriptor_1"] = json_to_descriptor;

  // Publish the combined JSON to MQTT
  mqtt::message_ptr msg =
      mqtt::make_message(input_topic_, combined_json.dump());
  msg->set_qos(1);
  client_.publish(msg)->wait();

  // wait until receive result
  while (matches_.empty()) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  matches = matches_;
  matches_.clear();
}

void MqttMatcher::message_arrived(mqtt::const_message_ptr msg) {
  std::cout << __FILE__ << " " << __FUNCTION__ << " " << __LINE__
            << " Received msg" << std::endl;

  matches_.clear();
  auto payload = msg->to_string();
  // Parse JSON data
  auto json_matches = nlohmann::json::parse(payload);

  // Convert JSON data to std::vector<cv::DMatch>
  for (const auto &match : json_matches) {
    int queryIdx = match[0];
    int trainIdx = match[1];
    matches_.emplace_back(queryIdx, trainIdx, 0.0);
  }
}
} // namespace rtabmap
