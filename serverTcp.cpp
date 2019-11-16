#include <cstring>
#include <vector>
#include <algorithm>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include "lib/config.hpp"
#include "lib/utils.hpp"

bool isNegative(struct pollfd c) { return ((c.fd == -1)); }

int main(int argc, char *argv[]) {
  char *port = argv[1];
  char client_message[MAX_LEN];
  struct pollfd client[MAX_CLIENT];
  Config *tcp_config = new Config();
  char addr[] = "::";  

  int my_socket = create_socket(port, false, addr);
  if (my_socket == -1) {
    print_error_and_exit("binded socket creation failed");
  }

  int enable = 1;
  if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    print_error_and_exit("setsockopt(SO_REUSEADDR) failed");

  client[0].fd = my_socket;  // 0 é o servidor
  client[0].events = POLLRDNORM; // há dados para serem lidos  
  for (int i = 1; i < MAX_CLIENT; ++i) {
    client[i].fd = -1; // entrada não está sendo usada
  }  
  
  while (true) {    
    int readable_desc = poll(client, MAX_CLIENT, -1); // esperar para sempre    
    
    if (client[0].revents & POLLRDNORM) { // nova conexão      
      int new_client = accept(my_socket, NULL, NULL);      
      if (new_client == tcp_config->error_state) {
        print_error_and_exit("new client with error");
      }
      auto begin = client + 1;
      auto end = client + MAX_CLIENT;      
      auto p = std::find_if (begin, end, isNegative);

      if (p == end) {
        close(new_client); // servido está cheio!
        continue;
      }
      p->fd = new_client;
      p->events = POLLRDNORM; // nova conexão pronta pare ser aceita
      p->revents = 0;
      if (--readable_desc <= 0) {
        continue;          
      }
    }
    
    for (int i = 1; i < MAX_CLIENT; ++i) {
      if (client[i].fd == -1) {
        continue;
      }
      else{
        std::cout << "else" << "\n";
      }
      std::cout << "hihi" << "\n";
      
      if (client[i].revents & POLLIN) { // RST foi recebido
        std::cout << "pollin!" << "\n";
        int received = recv(
          client[i].fd,
          client_message,
          sizeof(client_message)-1,
          tcp_config->rec_flags);
        
        if (received == tcp_config->error_state) {
          print_error_and_exit("received error");
        }
        std::cout << "received" << "\n";
        
        std::cout << client_message << "\n";
        
        if (received <= 0) {          
          printf("Client closed or aborted connection!");
          close(client[i].fd);
          client[i].fd = -1;
          
          continue;
        }
        
        write(client[i].fd, client_message, received);
        
        if (--readable_desc <= 0) {
          break;          
        }        
      }
    }
  }
  exit(0);
}
