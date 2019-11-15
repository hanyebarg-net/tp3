#include <iostream>
#include <cstring>
#include <map>
#include "config.hpp"
#include "utils.hpp"
#define MAXSIZE 80


void print_error_and_exit(std::string message) {
  std::cout << message << "\n";
  exit(1);
}

Addr create_and_initialize_address() { // generic for v4 or v6
  Config* config = new Config();
  Addr address;
  std:: memset(&address, 0, sizeof(address));
  address.ai_family = config->ai_family;
  address.ai_socktype = config->sock_type;
  address.ai_protocol = config->protocol;

  return address;
}

Addr create_and_initialize_address_UDP(bool is_client) { // generic for v4 or v6
  Config* config = new Config();
  Addr address;
  std:: memset(&address, 0, sizeof(address));
  address.ai_family = config->ai_family;
  address.ai_socktype = config->sock_type_UDP;
  address.ai_protocol = config->protocol_UDP;
  if (!is_client) {
    address.ai_flags = config->ai_flags;
  }

  return address;
}

char* invert_case(char *str, int num_bytes) {
  for (int i = 0; i < num_bytes; ++i) {
      if (std::isupper(str[i])) {
        str[i] = std::tolower(str[i]);
      }
      else {
        str[i] = std::toupper(str[i]);
      }
  }
  return str;
}

int create_socket(char *port, bool is_client, char *host) {
  Config* config = new Config();
  Addr address_holder = create_and_initialize_address();
  Addr *server_address;

  if (!is_client) {
    address_holder.ai_flags = config->ai_flags;
  }
  // getaddrinfo() returns 0 if it succeeds
  if (getaddrinfo(host, port, &address_holder, &server_address) != 0) {
    print_error_and_exit("Getaddrinfo failed");
  };
  
  int my_sock = -1;
  my_sock = socket(server_address->ai_family, server_address->ai_socktype, server_address->ai_protocol);
  if (my_sock == -1) {
    print_error_and_exit("socket creation failed");
  }
  if (is_client && connect(my_sock, server_address->ai_addr, server_address->ai_addrlen) == -1) {    
    print_error_and_exit("connecion failed... returning -1");
    my_sock = -1;
  }
  else if(
    is_client == false &&
    (bind(my_sock, server_address->ai_addr, server_address->ai_addrlen) == -1 ||
     listen(my_sock, config->backlog) == -1)
    ) {        
    print_error_and_exit("bind or listen failed... returning -1");
    my_sock = -1;  
  }

  freeaddrinfo(server_address);
  return my_sock;
}
