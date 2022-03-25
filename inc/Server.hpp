#ifndef SERVER_HPP
# define SERVER_HPP

#include "ClientsMonitoringList.hpp"

#include <iostream>
#include <vector>

#include <poll.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#include <stdio.h>
#include <string.h>
#include <string.h>
#include <sys/errno.h>
#include <unistd.h>

#define ERROR -1 // je trouve -1 plus logique perso
#define PROTO "TCP"
#define PORT 16385
#define SOCK_DOMAIN AF_INET
#define IP_ADDR "127.0.0.1"
#define SOCK_TYPE SOCK_STREAM
#define RECV_BUF_SIZE 1024

class Server
{
	private:

		int const			_port;
		std::string const	_password;
		Server();

	public:

		Server(int const port, std::string const password);
		Server(Server const &instance);
		~Server();
		Server &operator=(Server const &instance);

		int	launch(void); // const ?
		int	create_server_descriptor(void) const;
		int	monitor_clients(int server_fd); // const ?
		void add_descriptor_to_poll(int fd, ClientsMonitoringList *Clients, struct pollfd *pfds, nfds_t &nb_pfds); // const ?
		void remove_descriptor_from_poll(ClientsMonitoringList &Client, struct pollfd &pfds, nfds_t &nb_pfds);

};

#endif