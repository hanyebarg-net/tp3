#include <sys/types.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


// A class with the socket configuration for ipv4 and ipv6
class Config
{
  public:
  int error_state;

  // socket
  int domain;
  int sock_type;
  int protocol;

  // server address
  int ai_family;
  int ai_flags;

  // listen
  int backlog; // maximum length to which the queue of pending connections may grow

  // receive
  int rec_flags;

  // send
  int send_flags;

  // UDP
  int sock_type_UDP;
  int protocol_UDP;

  // unicode chars
  char SOH;
  char STX;
  char ETX;
  char EOT;
  char ENQ;
  char ACK;
  char PM;
  
  Config();
};
