#include "config.hpp"

Config ::Config()
    : error_state(-1),
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
