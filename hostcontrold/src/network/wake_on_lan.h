#pragma once

#include "wol_interface.h"

class WakeOnLan : public WolInterface {
 public:
  bool SendMagicPacket(const std::string& mac_address) const override;

 private:
  static bool SendWol(const std::string& mac);

  /// Input an Ethernet address and convert to binary.
  static bool ConvertMacStringToBinary(const std::string& mac_string, unsigned char* mac_binary);
};
