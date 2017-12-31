#ifndef UTILS_SD_JOURNAL_LOGGER_CORE_H_
#define UTILS_SD_JOURNAL_LOGGER_CORE_H_

/**
 * \brief Base class that contains some core functionality that is central to SdJournalLogger.
 */
class SdJournalLoggerCore {
 public:
  /// \brief Allow to limit the log messages to maximum log level that should be logged. All higher levels are ignored.
  static void SetMaxLogPriority(int max) {
    max_priority_ = max;
  }

 private:
  static int max_priority_;
};

#endif  // UTILS_SD_JOURNAL_LOGGER_CORE_H_
