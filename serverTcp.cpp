#include <cstddef>
#include <cstring>
#include <string>
#include <sys/poll.h>
#include <vector>
#include <map>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include "lib/config.hpp"
#include "lib/utils.hpp"

bool isNegative(struct pollfd c) { return ((c.fd == -1)); }

auto concat = [](std::string &a, std::string &b) {
                         return a + '\n' + b;
              };

int main(int argc, char *argv[]) {
  char *port = argv[1];
  std::vector<std::string> user_names;
  std::map<std::string, int> catalogue;
  std::string user_names_list;

  std::vector<pollfd> sockets;
  Config *tcp_config = new Config();
  char addr[] = "::";

  int my_socket = create_socket(port, false, addr);
  if (my_socket == -1) {
    print_error_and_exit("binded socket creation failed");
  }

  int enable = 1;
  if (setsockopt(my_socket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    print_error_and_exit("setsockopt(SO_REUSEADDR) failed");

  sockets.push_back({my_socket, POLLRDNORM});
  for (int i = 1; i < MAX_CLIENT; ++i) {
    sockets.push_back({-1});
  }

  while (true) {
    int readable_desc = poll(sockets.data(), MAX_CLIENT, -1); // esperar para sempre

    if (sockets[0].revents & POLLRDNORM) { // nova conexão
      int new_client = accept(my_socket, NULL, NULL);
      if (new_client == tcp_config->error_state) {
        print_error_and_exit("new client with error");
      }
      auto begin = sockets.begin() + 1;
      auto end = sockets.begin() + MAX_CLIENT;
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
      if (sockets[i].fd == -1) {
        continue;
      }

      if (sockets[i].revents & (POLLRDNORM | POLLERR)) { // RST foi recebido
        char client_message[MAX_LEN] = "";
        std::cout << "receiving" << "\n";
        int received = recv(sockets[i].fd, client_message,
                            sizeof(client_message) - 1, tcp_config->rec_flags);
        if (received == tcp_config->error_state) {
          std::cout << "client closed connection" << "\n";
        }
        if (client_message[0] == tcp_config->ACK) {          
          user_names.push_back(client_message);
          user_names_list += client_message;
          user_names_list += '\n';

          catalogue[client_message] = i;
        }
        else if (client_message[0] == tcp_config->ENQ) {
          const char *client_message = user_names_list.c_str();
          write(sockets[i].fd, client_message, sizeof(user_names_list));
        }
        else if (client_message[0] == tcp_config->STX) {          
          for (int j = 1; j < MAX_CLIENT; j++) {
            if (sockets[j].fd != -1) {
              std::cout << j << "\n";
              write(sockets[j].fd, client_message, sizeof(client_message));          
            }
          }
        }        
        if (received <= 0) {
          std::cout << "saiu" << "\n";
          
          close(sockets[i].fd);
          sockets[i].fd = -1;

          if (sockets.size() < 2) { // só tem um cliente
            sockets.clear();
            user_names.clear();            
            continue;
          }

          std::swap(sockets[i], sockets.back());
          sockets.pop_back();
          
          std::swap(user_names[i], user_names.back());
          user_names.pop_back();

          user_names_list = std::accumulate(user_names.begin(), user_names.end(), std::string{});
          
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
