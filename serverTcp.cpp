#include <cstring>
#include <vector>
#include <algorithm>
#include <poll.h>
#include "lib/config.hpp"
#include "lib/utils.hpp"


int main(int argc, char *argv[]) {
  char *port = argv[1];
  char client_message[MAX_LEN];
  struct pollfd client[MAX_CLIENT];
  Config *tcp_config = new Config();
  char addr[] = "::";
  int max_index = 0; // indice mais alto do array de clientes em uso

  int my_socket = create_socket(port, false, addr);
  if (my_socket == -1) {
    print_error_and_exit("binded socket creation failed");
  }

  int enable = 1;
  if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    print_error_and_exit("setsockopt(SO_REUSEADDR) failed");

  client[0].fd = my_socket;
  client[0].events = POLLIN; // há dados para serem lidos  
  
  for (int i = 0; i < MAX_CLIENT; ++i) {
    client[i].fd = -1; // entrada não está sendo usada
  }
  max_index = 1;
  
  while (true) {// Wait for a client to connect forever
    poll(client, max_index, -1); // esperar para sempre
    if (client[0].revents && POLLIN) { // nova conexão
      int new_client = accept(my_socket, NULL, NULL);      
      if (new_client == tcp_config->error_state) {
        print_error_and_exit("new client with error");
      }
      
    }
    
  }
  exit(0);
}
