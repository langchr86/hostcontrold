/*****************************************************************************
 *
 * wol.c - Wake-On-LAN utility to wake a networked PC
 *
 * by R. Edwards (bob@cs.anu.edu.au), January 2000
 * (in_ether routine adapted from net-tools-1.51/lib/ether.c by
 * Fred N. van Kempen)
 * 
 * This utility allows a PC with WOL configured to be powered on by
 * sending a "Magic Packet" to it's network adaptor (see:
 * http://www.amd.com/products/npd/overview/20212.html).
 * Only the ethernet MAC address needs to be given to make this work.
 *
 * Current version uses a UDP broadcast to send out the Magic Packet.
 * A future version will use the raw packet interface to unicast the
 * packet only to the target machine.
 *
 * compile with: gcc -Wall -o wol wol.c
 * 
 * usage: wol <MAC address>
 * where <MAC address> is in xx:xx:xx:xx:xx:xx format.
 *
 * Released under GNU Public License January, 2000.
 */

#include "wake_on_lan.h"

#include <linux/if_ether.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstring>
#include <unistd.h>

#include "utils/sd_journal_logger.hpp"

static const SdJournalLogger<>& GetLogger() {
  static SdJournalLogger<> logger(__FILE__, "WakeOnLan", {});
  return logger;
}

bool WakeOnLan::SendMagicPacket(const std::string& mac_address) const {
  return SendWol(mac_address);
}

bool WakeOnLan::SendWol(const std::string& mac) {
  int i, j;
  int packet;
  struct sockaddr_in sap;
  unsigned char ethaddr[8];
  unsigned char* ptr;
  unsigned char buf[128];
  int optval = 1;

  /* Fetch the hardware address. */
  if (ConvertMacStringToBinary(mac, ethaddr) == false) {
    GetLogger().SdLogErr("Invalid hardware address: %s", mac.c_str());
    return false;
  }

  /* setup the packet socket */
  if ((packet = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
    GetLogger().SdLogErr("Socket failed.");
    return false;
  }

  /* Set socket options */
  if (setsockopt(packet, SOL_SOCKET, SO_BROADCAST, (caddr_t) &optval, sizeof(optval)) < 0) {
    GetLogger().SdLogErr("setsocket failed: %s", std::strerror(errno));
    close(packet);
    return false;
  }

  /* Set up broadcast address */
  sap.sin_family = AF_INET;
  sap.sin_addr.s_addr = htonl(0xffffffff);        /* broadcast address */
  sap.sin_port = htons(60000);

  /* Build the message to send - 6 x 0xff then 16 x MAC address */
  ptr = buf;
  for (i = 0; i < 6; i++)
    *ptr++ = 0xff;
  for (j = 0; j < 16; j++)
    for (i = 0; i < ETH_ALEN; i++)
      *ptr++ = ethaddr[i];

  /* Send the packet out */
  if (sendto(packet, buf, 102, 0, (struct sockaddr*) &sap, sizeof(sap)) < 0) {
    GetLogger().SdLogErr("sendto failed, %s", std::strerror(errno));
    close(packet);
    return false;
  }

  close(packet);
  return true;
}

bool WakeOnLan::ConvertMacStringToBinary(const std::string& mac_string, unsigned char* mac_binary) {
  char c;
  auto read_iterator = mac_string.cbegin();
  unsigned val;

  int i = 0;

  while ((*read_iterator != '\0') && (i < ETH_ALEN)) {
    val = 0;
    c = *read_iterator++;
    if (isdigit(c)) {
      val = static_cast<unsigned int>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      val = static_cast<unsigned int>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      val = static_cast<unsigned int>(c - 'A' + 10);
    } else {
      GetLogger().SdLogErr("Invalid ether address: %s", mac_string.c_str());
      return false;
    }

    val <<= 4;
    c = *read_iterator;

    if (isdigit(c)) {
      val |= static_cast<unsigned int>(c - '0');
    } else if (c >= 'a' && c <= 'f') {
      val |= static_cast<unsigned int>(c - 'a' + 10);
    } else if (c >= 'A' && c <= 'F') {
      val |= static_cast<unsigned int>(c - 'A' + 10);
    } else if (c == ':' || c == 0) {
      val >>= 4;
    } else {
      GetLogger().SdLogErr("Invalid ether address: %s", mac_string.c_str());
      return false;
    }

    if (c != 0) {
      read_iterator++;
    }

    *mac_binary++ = (unsigned char) (val & 0377);
    i++;

    /* We might get a semicolon here - not required. */
    if (*read_iterator == ':') {
      if (i == ETH_ALEN) {
        /* nothing */
      }
      read_iterator++;
    }
  }
  return true;
}
