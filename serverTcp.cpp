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

  while (true) {
    poll(sockets.data(), sockets.size(), -1); // esperar para sempre

    if (sockets[0].revents & POLLRDNORM) { // nova conexão
      int new_client = accept(my_socket, NULL, NULL);

      if (new_client == tcp_config->error_state) {
        print_error_and_exit("new client with error");
      }

      sockets.push_back({new_client, POLLRDNORM});
      user_names.push_back("undefined");
    }

    auto socket = sockets.begin() + 1;

    while (socket != sockets.end()) {
      if (socket->revents & (POLLRDNORM | POLLERR)) {
        std::string client_message;
        client_message.resize(MAX_LEN);

        int received = recv(
          socket->fd,
          client_message.data(),
          MAX_LEN,
          tcp_config->rec_flags
        );

        if (received == tcp_config->error_state) {
          std::cout << "erro!\n";
          return -1;
        }

        client_message.resize(received);

        if (received <= 0) { // usuário saiu
          close(socket->fd);

          if (user_names.size() < 2) { // só tem um cliente
            user_names.clear();
            catalogue.clear();
            sockets.erase(sockets.begin() + 1);
            break;
          }

          auto i = socket - sockets.begin();

          catalogue.erase(user_names[i - 1]);


          std::swap(user_names[i - 1], user_names.back());
          user_names.pop_back();

          auto end = sockets.end();

          std::iter_swap(socket, sockets.end() - 1);
          sockets.pop_back();

          if (socket + 1 == end)
            break;

          catalogue[user_names[i - 1]] = i;

          continue;
        }

        if (client_message[0] == tcp_config->ACK) {
          auto name = client_message.substr(1);

          if (catalogue.count(name)) {
            char message[MAX_LEN] = "";
            message[0] = tcp_config->SOH;
            strcat(message, "Usuário indisponível\0");
            write(socket->fd, message, strlen(message));
          }
          else {
            auto i = socket - sockets.begin();

            catalogue[name] = i;
            user_names[i - 1] = name;
          }
        }
        else if (client_message[0] == tcp_config->ENQ) { // listar usuários
          std::string response;

          for (auto& username : user_names)
            response += username + "\n";

          write(socket->fd, response.data(), response.size());
        }
        else if(client_message[0] == tcp_config->PM) {
          std::size_t first = client_message.find(';');
          std::size_t last = client_message.find_last_of(';');

          std::string msg = client_message.substr(last+1);
          std::string user_pm = client_message.substr(first+1, last-2);
          std::cout << user_pm << "\n";

          if (catalogue.find(user_pm) == catalogue.end() ) {
            const char response[] = "Usuário não encontrado!";
            write(socket->fd, response, strlen(response));
          }
          else {
            std::size_t sock_position = catalogue[user_pm];
            write(sockets[sock_position].fd, msg.data(), msg.size());
          }
        }
        else if (client_message[0] == tcp_config->STX) { // broadcast
          auto text = client_message.substr(1);

          for (auto socket = sockets.begin() + 1; socket != sockets.end(); ++socket) {
            write(socket->fd, text.data(), text.size());
          }
        }
      }

      socket++;
    }
  }
  return 0;
}
