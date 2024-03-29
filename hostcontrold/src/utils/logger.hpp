#pragma once

#include <sys/syslog.h>

#include <cstddef>

#include <string>
#include <tuple>
#include <utility>
#include <array>
#include <regex>

#include "utils/logger_core.h"

/**
 * \brief Logging wrapper for formatting and logging to console.
 *
 * Wrapper to allow flexible logging with context information to console. Therefore we can instantiate this class
 * with static content information like class or file name and an arbitrary list of dynamic context data that is
 * evaluated at log time. Logs with DEBUG priority contain also some meta data in the log message.
 *
 * \tparam Context Parameter pack for all values used in the prefix string. Do provide only the exact type that is
 *                 pointed to and no extra const or *.
 */
template<typename... Context>
class Logger : private LoggerCore {
  // some helpers for tuple unpacking
  template<size_t...>
  struct seq {
  };

  template<size_t N, size_t... Is>
  struct gen_seq : gen_seq<N - 1, N - 1, Is...> {
  };

  template<size_t... Is>
  struct gen_seq<0, Is...> : seq<Is...> {
  };

 public:
  using StringArray = std::array<std::string, sizeof...(Context)>;
  using ContextTuple = std::tuple<const Context* ...>;

  /**
   * \brief Initialize a logger with context information.
   *
   * \attention The values pointed to by pointers need to be available until last call to Logger::Log.
   *
   * \param class_name The class name in which this logger instance is used.
   * \param context_formatters The format strings in an std::array. They should be of form: VAR_NAME=%i or similar.
   * \param context_pointers Pointers to the values that are dynamically inserted in the format string.
   */
  Logger(const std::string& class_name,
         const StringArray& context_formatters,
         const Context* ... context_pointers)
      : class_name_(class_name)
      , context_formatters_(context_formatters)
      , context_pointers_(context_pointers...) {
    // ensure correct formatters
    const std::regex expression("[A-Z_]*=%.*");
    for (const auto& format : context_formatters) {
      if (std::regex_match(format, expression) == false) {
        fprintf(stderr, "Context formatter \"%s\" does not match form: \"VAR_NAME=%%i\".", format.c_str());
      }
    }
  }

  /// \attention Do not allow to copy logger with pointers to possible old variables.
  Logger(const Logger&) = delete;
  /// \attention Do not allow to copy logger with pointers to possible old variables.
  Logger& operator=(const Logger&) = delete;

  /**
   * \brief Do the logging with the log message formatter string and the arguments.
   *
   * \param log_format The message format string.
   * \param args All arguments that should be represented in the message string.
   */
  template<typename... Args>
  void LogFunc(const char* const function_name, int priority,
                 const char* const log_format, Args&& ... args) const {
    if (priority > max_priority_) {
      return;
    }
    InternLog(gen_seq<sizeof...(Context)>(),
              function_name,
              priority,
              log_format,
              std::forward<Args>(args)...);
  }

 private:
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
  template<typename... Args, size_t... Is>
  void InternLog(seq<Is...>,
                 const char* const function_name,
                 int priority,
                 const char* const log_format,
                 Args&& ... args) const {
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

    FILE* log_stream = stdout;
    if (priority <= LOG_ERR) {
      log_stream = stderr;
    }

    const auto message_format = std::string("[%s][%s::%s](%s) ") + log_format + "\n";
    fprintf(log_stream,
            message_format.c_str(),
            GetLogLevelText(priority),
            class_name_.c_str(),
            function_name,
            overall_dynamic_context_string.c_str(),
            std::forward<Args>(args)...);
    fflush(log_stream);
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

#define Log(...) LogFunc(__func__, __VA_ARGS__)

#define LogDebug(...)   Log(LOG_DEBUG, __VA_ARGS__)
#define LogInfo(...)    Log(LOG_INFO, __VA_ARGS__)
#define LogNotice(...)  Log(LOG_NOTICE, __VA_ARGS__)
#define LogWarning(...) Log(LOG_WARNING, __VA_ARGS__)
#define LogErr(...)     Log(LOG_ERR, __VA_ARGS__)
#define LogCrit(...)    Log(LOG_CRIT, __VA_ARGS__)
#define LogAlert(...)   Log(LOG_ALERT, __VA_ARGS__)
#define LogEmerg(...)   Log(LOG_EMERG, __VA_ARGS__)
