#pragma once

#include "rtabmap/core/mqtt/mqtt_utils.hpp"
#include "rtabmap/utilite/ULogger.h"

#include <mqtt/client.h>
#include <opencv2/core/types.hpp>
#include <vector>

namespace rtabmap {
class MqttMatcher : public virtual mqtt::callback,
                    public virtual mqtt::iaction_listener {
public:
  MqttMatcher(std::string_view server_addrs, std::string_view input_topic,
              std::string_view output_topic);
  // MqttMatcher(MqttMatcher &&) = default;
  // MqttMatcher(const MqttMatcher &) = default;
  // MqttMatcher &operator=(MqttMatcher &&) = default;
  // MqttMatcher &operator=(const MqttMatcher &) = default;
  ~MqttMatcher() override = default;

  // mqtt callback and data handler
  void connection_lost(const std::string &cause) override {
    std::cerr << "Connection lost: " << cause << std::endl;
    exit(EXIT_FAILURE);
  }

  void delivery_complete(mqtt::delivery_token_ptr token) override {}

  void on_failure(const mqtt::token &tok) override {
    UERROR("MQTT failure: %d", tok.get_message_id());
    exit(EXIT_FAILURE);
  }

  void on_success(const mqtt::token &tok) override {
    if (tok.get_message_id() != 0) {
      UINFO("MQTT success");
    }
  }

  void message_arrived(mqtt::const_message_ptr msg) override;

  void request_matching(const std::vector<cv::KeyPoint> &from_kps,
                        const cv::Mat &from_desc,
                        const std::vector<cv::KeyPoint> &to_kps,
                        const cv::Mat &to_desc,
                        std::vector<cv::DMatch> &matches);

private:
  // mqtt
  mqtt::async_client client_;
  std::string input_topic_;

  std::vector<cv::DMatch> matches_{};
};
} // namespace rtabmap
