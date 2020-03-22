#pragma once

#include <string>

/**
 * \brief Base class that contains some core functionality that is central to SdJournalLogger.
 */
class WakeOnLan {
 public:
  static bool SendWol(const std::string& mac);

 private:
  /// Input an Ethernet address and convert to binary.
  static bool ConvertMacStringToBinary(const std::string& mac_string, unsigned char* mac_binary);
};
