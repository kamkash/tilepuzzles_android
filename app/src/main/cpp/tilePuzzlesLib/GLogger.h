#ifndef _GLOGGER_H_
#define _GLOGGER_H_

#include <glog/logging.h>
namespace tilepuzzles {

struct Logger {

  static void initLogger(const char* modName) {
    google::InitGoogleLogging(modName);
  }

  static constexpr Logger getLogger() {
    return Logger();
  }

  template <class Head>
  void log_(std::ostream& s, Head&& head) const {
    s << std::forward<Head>(head);
    // google::FlushLogFiles(google::INFO);
  }

  template <class Head, class... Tail>
  void log_(std::ostream& s, Head&& head, Tail&&... tail) const {
    s << std::forward<Head>(head) << " ";
    log_(s, std::forward<Tail>(tail)...);
  }

  template <class... Args>
  void info(Args&&... args) const {
    log_(LOG(INFO), std::forward<Args>(args)...);
  }

  template <class... Args>
  void warn(Args&&... args) const {
    log_(LOG(WARNING), std::forward<Args>(args)...);
  }

  template <class... Args>
  void error(Args&&... args) const {
    log_(LOG(ERROR), std::forward<Args>(args)...);
  }

  template <class... Args>
  void fatal(Args&&... args) const {
    log_(LOG(FATAL), std::forward<Args>(args)...);
  }

  template <class... Args>
  void debug(Args&&... args) const {
    log_(DLOG(INFO), std::forward<Args>(args)...);
  }
};
} // namespace tilepuzzles

#endif