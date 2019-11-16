#include "config.hpp"

Config ::Config()
    : error_state(-1),      
      SOH(0x01),
      STX(0x02),
      ETX(0x03),
      EOT(0x04),
      ENQ(0x05),
      ACK(0x06),
      PM(0x9E),
      domain(AF_INET),
      sock_type(SOCK_STREAM),
      protocol(IPPROTO_TCP),
      ai_family(AF_UNSPEC),
      ai_flags(AI_PASSIVE),
      backlog(5),
      rec_flags(0),
      send_flags(0),
      sock_type_UDP(SOCK_DGRAM),
      protocol_UDP(IPPROTO_UDP){}
