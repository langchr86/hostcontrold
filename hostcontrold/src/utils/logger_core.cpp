#include "logger_core.h"

#include <sys/syslog.h>

int LoggerCore::max_priority_ = LOG_DEBUG;
