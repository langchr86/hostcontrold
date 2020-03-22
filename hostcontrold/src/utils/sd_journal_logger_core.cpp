#include "sd_journal_logger_core.h"

#include <sys/syslog.h>

int SdJournalLoggerCore::max_priority_ = LOG_DEBUG;
