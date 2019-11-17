#include <cstddef>
#include <cstring>
#include <vector>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include "lib/config.hpp"
#include "lib/utils.hpp"

bool isNegative(struct pollfd c) { return ((c.fd == -1)); }

int main(int argc, char *argv[]) {
  char *port = argv[1];
  std::vector<std::string> user_names;
  std::vector<std::string> user_names_table;
  std::string user_names_list;


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

      if (client[i].revents & (POLLRDNORM | POLLERR)) { // RST foi recebido
        char client_message[MAX_LEN] = "";        
        int received = recv(
          client[i].fd,
          client_message,
          sizeof(client_message)-1,
          tcp_config->rec_flags);

        if (received == tcp_config->error_state) {
          std::cout << "client closed connection" << "\n";
        }
        if (client_message[0] == tcp_config->ACK) {
          user_names_table.push_back(client_message);
          
          user_names.push_back(client_message);
          user_names_list += client_message;
          user_names_list += '\n';
        }
        else if (client_message[0] == tcp_config->ENQ) {
          const char *client_message = user_names_list.c_str();          
          write(client[i].fd, client_message, sizeof(user_names_list));
        }

        if (received <= 0) {
          std::cout << "segundo saiu" << "\n";
          close(client[i].fd);
          client[i].fd = -1;
          
          user_names.erase(std::find(user_names.begin(), user_names.end(), user_names_table[i]));
          user_names_list = std::accumulate(user_names.begin(), user_names.end(), std::string("\n"));
          user_names_list += '\n';
          
          continue;
        }

        if (--readable_desc <= 0) {
          break;
        }
      }
    }
  }
  exit(0);
}
