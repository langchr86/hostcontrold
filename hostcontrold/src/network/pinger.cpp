#include "pinger.h"

#include <oping.h>

class PingGuard {
 public:
  explicit PingGuard(pingobj_t* obj)
      : obj_(obj) {}
  ~PingGuard() { ping_destroy(obj_); }
 private:
  pingobj_t* obj_;
};

Pinger::Pinger()
    : logger_(__FILE__, "Pinger", {}) {}

PingResult Pinger::PingHost(const std::string& ip) const {
  // create object
  pingobj_t* obj = ping_construct();
  PingGuard guard(obj);

  // add host to object
  if (ping_host_add(obj, ip.c_str()) < 0) {
    logger_.SdLogErr("failed to parse provided address '%s'", ip.c_str());
    return PingResult::kFailed;
  }

  // send ICMP
  const int res = ping_send(obj);
  if (res < 0) {
    logger_.SdLogErr("failed to send ICMP to address '%s': %s", ip.c_str(), ping_get_error(obj));
    return PingResult::kFailed;
  } else if (res == 0) {
    return PingResult::kHostInactive;
  }

  // receive info
  pingobj_iter_t* iter = ping_iterator_get(obj);
  double latency = -1.0;
  size_t buffer_len = sizeof(latency);

  if (iter == nullptr) {
    logger_.SdLogErr("failed to get object iterator for address '%s'", ip.c_str());
    return PingResult::kFailed;
  }

  if (ping_iterator_get_info(iter, PING_INFO_LATENCY, &latency, &buffer_len) < 0) {
    logger_.SdLogErr("failed to get latency information for address '%s'", ip.c_str());
    return PingResult::kFailed;
  }

  // return result
  if (latency < 0.0) {
    return PingResult::kHostInactive;
  } else {
    return PingResult::kHostActive;
  }
}
