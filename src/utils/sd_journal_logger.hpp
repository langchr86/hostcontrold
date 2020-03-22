#ifndef UTILS_SD_JOURNAL_LOGGER_HPP_
#define UTILS_SD_JOURNAL_LOGGER_HPP_

#include <systemd/sd-journal.h>

#include <sys/syslog.h>

#include <string>
#include <tuple>
#include <utility>
#include <array>
#include <regex>

#include "utils/sd_journal_logger_core.h"

/**
 * \brief Logging wrapper for sd-journal.
 *
 * Wrapper to allow flexible logging with context information to journald. Therefore we can instantiate this class
 * with static content information like class or file name and an arbitrary list of dynamic context data that is
 * evaluated at log time. All these information are logged as meta data into the log event ready to be filtered by
 * journalctl. Logs with DEBUG priority contain also some meta data in the log message.
 *
 * See unit test for usage examples.
 *
 * \tparam Context Parameter pack for all values used in the prefix string. Do provide only the exact type that is
 *                 pointed to and no extra const or *.
 */
template<typename... Context>
class SdJournalLogger : private SdJournalLoggerCore {
  // some helpers for tuple unpacking
  template<int...>
  struct seq {
  };

  template<int N, int... Is>
  struct gen_seq : gen_seq<N - 1, N - 1, Is...> {
  };

  template<int... Is>
  struct gen_seq<0, Is...> : seq<Is...> {
  };

 public:
  using StringArray = std::array<std::string, sizeof...(Context)>;
  using ContextTuple = std::tuple<const Context* ...>;

  /**
   * \brief Initialize a logger with context information.
   *
   * \attention The values pointed to by pointers need to be available until last call to SdJournalLogger::Log.
   *
   * \param file_name The file name in which this class is defined. Use macro: __FILE__
   * \param class_name The class name in which this logger instance is used.
   * \param context_formatters The format strings in an std::array. They should be of form: VAR_NAME=%i or similar.
   *                           See sd_journal_send for information about formatting of journal meta fields.
   * \param context_pointers Pointers to the values that are dynamically inserted in the format string.
   */
  SdJournalLogger(const std::string& file_name,
                  const std::string& class_name,
                  const StringArray& context_formatters,
                  const Context* ... context_pointers)
      : file_name_(file_name)
      , class_name_(class_name)
      , context_formatters_(context_formatters)
      , context_pointers_(context_pointers...) {
    // ensure correct formatters
    const std::regex expression("[A-Z_]*=%.*");
    for (const auto& format : context_formatters) {
      if (std::regex_match(format, expression) == false) {
        sd_journal_print(LOG_ERR, "Context formatter \"%s\" does not match form: \"VAR_NAME=%%i\".", format.c_str());
      }
    }
  }

  /// \attention Do not allow to copy logger with pointers to possible old variables.
  SdJournalLogger(const SdJournalLogger&) = delete;
  /// \attention Do not allow to copy logger with pointers to possible old variables.
  SdJournalLogger& operator=(const SdJournalLogger&) = delete;

  /**
   * \brief Do the logging with the log message formatter string and the arguments.
   *
   * \param log_format The message format string.
   * \param args All arguments that should be represented in the message string.
   */
  template<typename... Args>
  void SdLogFunc(const char* const function_name, const char* const line_number, int priority,
                 const char* const log_format, Args&& ... args) const {
    if (priority > max_priority_) {
      return;
    }
    InternLog(gen_seq<sizeof...(Context)>(),
              function_name,
              line_number,
              priority,
              log_format,
              std::forward<Args>(args)...);
  }

 private:
  const std::string file_name_;
  const std::string class_name_;
  const StringArray context_formatters_;
  const ContextTuple context_pointers_;

  /**
   * \brief Internal working method to prepare real format string, evaluate all values and print.
   *
   * This uses the seq param (index trick) to unpack and therefore query all values in the tuple. The
   * EvaluateValue function is chosen to the matching type and reads the real value to print. In the usual case this
   * works by dereferencing the pointer.
   */
  template<typename... Args, int... Is>
  void InternLog(seq<Is...>,
                 const char* const function_name,
                 const char* const line_number,
                 int priority,
                 const char* const log_format,
                 Args&& ... args) const {
    // prepare static context strings
    const auto file_string = std::string("CODE_FILE=") + file_name_;
    const auto line_string = std::string("CODE_LINE=") + line_number;

    // format each individual dynamic context string and store it in an array
    StringArray dynamic_context_strings{
        FormatMetaString(context_formatters_[Is].c_str(), std::get<Is>(context_pointers_))...
    };

    // combine all dynamic context strings to one that is used in the message
    std::string overall_dynamic_context_string;
    const size_t number_of_meta_strings = dynamic_context_strings.size();
    for (size_t i = 0; i < number_of_meta_strings; ++i) {
      overall_dynamic_context_string += dynamic_context_strings[i];
      if (i < number_of_meta_strings - 1) {
        overall_dynamic_context_string += ", ";
      }
    }

    const auto message_format = std::string("MESSAGE=[%s][%s::%s](%s) ") + log_format;
    sd_journal_send_with_location(file_string.c_str(),
                                  line_string.c_str(),
                                  function_name,
                                  "PRIORITY=%i", priority,
                                  "CODE_CLASS=%s", class_name_.c_str(),
                                  dynamic_context_strings[Is].c_str()...,
                                  message_format.c_str(),
                                  GetLogLevelText(priority), class_name_.c_str(), function_name,
                                  overall_dynamic_context_string.c_str(),
                                  std::forward<Args>(args)...,
                                  NULL);
  }

  /// \brief Print evaluated value into formatting string and return new string object.
  template<typename T>
  static std::string FormatMetaString(const std::string& format, T&& value_pointer) {
    constexpr std::size_t kStringLength = 256;
    char formated_string[kStringLength];
    snprintf(formated_string, kStringLength, format.c_str(), EvaluateValue(std::forward<T>(value_pointer)));
    return formated_string;
  }

  /// \brief Evaluation function for generic pointer types.
  template<typename Type>
  static auto EvaluateValue(const Type* value_pointer) {
    return *value_pointer;
  }

  /// \brief Evaluation function for std::string.
  static auto EvaluateValue(const std::string* value_pointer) {
    return value_pointer->c_str();
  }

  /// \brief Evaluation function for C-style strings.
  static auto EvaluateValue(const char* value_pointer) {
    return value_pointer;
  }

  /// \brief Convert log priority to string.
  static const char* GetLogLevelText(int priority) {
    switch (priority) {
      case LOG_EMERG:
        return "EMER";
      case LOG_ALERT:
        return "ALER";
      case LOG_CRIT:
        return "CRIT";
      case LOG_ERR:
        return "ERRO";
      case LOG_WARNING:
        return "WARN";
      case LOG_NOTICE:
        return "NOTI";
      case LOG_INFO:
        return "INFO";
      case LOG_DEBUG:
        return "DEBU";
      default:
        return "UNKNOWN";
    }
  }
};

#define SdLog(...) SdLogFunc(__func__, _SD_STRINGIFY(__LINE__), __VA_ARGS__)

#define SdLogDebug(...)   SdLog(LOG_DEBUG, __VA_ARGS__)
#define SdLogInfo(...)    SdLog(LOG_INFO, __VA_ARGS__)
#define SdLogNotice(...)  SdLog(LOG_NOTICE, __VA_ARGS__)
#define SdLogWarning(...) SdLog(LOG_WARNING, __VA_ARGS__)
#define SdLogErr(...)     SdLog(LOG_ERR, __VA_ARGS__)
#define SdLogCrit(...)    SdLog(LOG_CRIT, __VA_ARGS__)
#define SdLogAlert(...)   SdLog(LOG_ALERT, __VA_ARGS__)
#define SdLogEmerg(...)   SdLog(LOG_EMERG, __VA_ARGS__)

#endif  // UTILS_SD_JOURNAL_LOGGER_HPP_
