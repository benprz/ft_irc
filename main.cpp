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

#include "Client.hpp"

#define ERROR -1 // je trouve -1 plus logique perso
#define PROTO "TCP"
#define PORT 16385
#define SOCK_DOMAIN AF_INET
#define IP_ADDR "127.0.0.1"
#define SOCK_TYPE SOCK_STREAM
#define RECV_BUF_SIZE 1024
#define PASSWORD "sylben123"

#define ERR_PASSWDMISMATCH 464

int	create_server_descriptor(void)
{
	int	server_fd, sockopt_reuseaddr_val;
    protoent *proto;
	struct sockaddr_in server_addr;

    // on récupère l'index du protocole TCP dans /etc/protocols
	// (sous UNIX seulement)
    if ((proto = getprotobyname(PROTO)) == NULL)
    {
        std::cout << "Couldn't find protocol: " << PROTO << std::endl;
        return (ERROR);
    }

    // on crée un socket sur le domaine d'internet
    if ((server_fd = socket(SOCK_DOMAIN, SOCK_TYPE, proto->p_proto)) == -1)
    {
        std::cout << "Server socket error -> socket() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

	bzero(&server_addr, sizeof(server_addr));
	server_addr.sin_family = SOCK_DOMAIN;
	server_addr.sin_port = htons(PORT);
	inet_pton(SOCK_DOMAIN, IP_ADDR, &server_addr.sin_addr);

    // setsockopt SO_REUSEADDR permet de réutiliser des adresses déjà utilisées
	// (ça permet de fix le fait que parfois les fd des sockets ne sont pas tout
	// le temps complètements supprimés, du coup ça met une erreur au moment du
	// bind())
    sockopt_reuseaddr_val = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &sockopt_reuseaddr_val, sizeof(sockopt_reuseaddr_val));

    // on bind l'adresse du serveur au socket
    if (bind(server_fd, (const struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	// faire cast c++ | il faudra unlink selon le man "Binding a name in the
	// UNIX domain creates a socket in the file system that must be deleted
	// by the caller when it is no longer needed (using unlink(2))."
    {
        std::cout << "Server socket error " << errno << " -> bind() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

    // on peut listen sur le socket, écouter les connexions entrantes
    if (listen(server_fd, MAX_CLIENT_CONNEXIONS) == -1)
    {
        std::cout << "Server socket error " << errno << " -> listen() : " << strerror(errno) << std::endl;
        return (ERROR);
    }

    // on set le server_fd en O_NONBLOCK pour que accept ne loop pas en
	// attendant une connexion et on surveille les I/O des sockets avec poll en
	// mettant le timeout à -1 pour que ça soit lui qui attende indéfiniment une
	// connexion
    if (fcntl(server_fd, F_SETFL, O_NONBLOCK) == -1)
    {
        std::cout << "Server socket error " << errno << " -> fcntl(*, F_SETFL, O_NONBLOCK) : " << strerror(errno) << std::endl;
        return (ERROR);
    }

	return (server_fd);
}

//overwrite sur un descriptor qui a ete remove (mis a -1)
/*void	add_descriptor_to_poll(int fd, ClientsMonitoringList *Clients, nfds_t *nb_clients)
{
	nfds_t i = 0;

	while (i < *nb_clients)
	{
		if (Clients[i].fd == -1)
			break ;
		i++;
	}
	Clients[i].fd = fd;
	Clients[i].events = POLLIN;
	Clients[i].revents = 0;
	*nb_clients += 1;
					std::cout << fd << " " << *nb_clients << std::endl;
}

void	remove_descriptor_from_poll(int fd, ClientsMonitoringList *Clients, nfds_t *nb_clients)
{
	for (nfds_t i = 0; i < *nb_clients; i++)
	{
		if (Clients[i].fd == fd)
		{
			Clients->reset();
			*nb_clients -= 1;
			break ;
		}
	}
}

void	send_msg_to_all_fds(struct pollfd *pfds, char *buf, ssize_t length, nfds_t nb_pfds, int sender_fd)
{
	std::cout << "sender_fd=" << sender_fd << std::endl;
	for (nfds_t i = 1; i < nb_pfds; i++)
	{
		if (sender_fd != pfds[i].fd)
			send(pfds[i].fd, buf, length, 0);
	}
}
*/

//overwrite sur un descriptor qui a ete remove (mis a -1)
void    add_descriptor_to_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
    pfds[*nb_pfds].fd = fd;
    pfds[*nb_pfds].events = POLLIN;
    pfds[*nb_pfds].revents = 0;
    *nb_pfds += 1;
}

void    remove_descriptor_from_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
    for (int i = 0; i < *nb_pfds; i++)
    {
        if (pfds[i].fd == fd)
        {
            pfds[i].fd = -1;
            pfds[i].events = 0;
            pfds[i].revents = 0;
            nb_pfds -= 1;
            break ;
        }
    }
}

void    send_msg_to_all_fds(struct pollfd *pfds, char *buf, ssize_t length, nfds_t nb_pfds, int sender_fd)
{
    std::cout << "sender_fd=" << sender_fd << std::endl;
    for (int i = 1; i < nb_pfds; i++)
    {
        if (sender_fd != pfds[i].fd)
            send(pfds[i].fd, buf, length, 0);
    }
}

std::vector<std::string> string_split(std::string s, const char delimiter)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);
    
    std::vector<std::string> output;
    
    while (end <= std::string::npos)
    {
	    output.__emplace_back(s.substr(start, end-start));

	    if (end == std::string::npos)
	    	break;

    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
    return output;
}

#define FOREACH_COMMAND(COMMAND) \
        COMMAND(PASS)   \
        COMMAND(NICK)	\
		COMMAND(NB_COMMANDS)

#define GENERATE_ENUM(ENUM) ENUM,
#define GENERATE_STRING(STRING) #STRING,

enum COMMAND_ENUM {
    FOREACH_COMMAND(GENERATE_ENUM)
};

static const char *g_commands_name[] = {
    FOREACH_COMMAND(GENERATE_STRING)
};

int	get_command_index(std::string command)
{
	for (int i = 0; i < NB_COMMANDS; i++)
	{
		if (command == g_commands_name[i])
			return (i);
	}
	return (-1);
}

void	pass_command(std::vector<std::string> split_packet)
{
	std::cout << "pass command!" << std::endl;
}

void	nick_command(std::vector<std::string> split_packet)
{
	std::cout << "nick command!" << std::endl;
}

void	(*g_commands_functions[NB_COMMANDS])(std::vector<std::string>) = {
	pass_command,
	nick_command
};

void	parse_client_packet(ClientsMonitoringList &Client, std::string packet)
{
	int	command_index;

	if (packet.find('\n') != std::string::npos)
	{
		packet.push_back('\n');
		packet.push_back('\r');
		std::cout << "A" << std::endl;
		std::vector<std::string> split_packet = string_split(packet, ' ');
		if (split_packet.size() > 0)
		{
			std::cout << "B " << packet << std::endl;
			if ((command_index = get_command_index(split_packet[0])) >= 0)
				g_commands_functions[command_index](split_packet);
			else
				send(Client.fd, "Error: unknown command", strlen("Error: unknown command"), 0);
		}
		else
			send(Client.fd, "Error: no input command", strlen("Error: no input command"), 0);
	}
	else
		Client.current_packet += packet;
}

int	monitor_clients(int server_fd)
{
	struct pollfd	Clients[MAX_CLIENT_CONNEXIONS];
	// ClientsMonitoringList	Clients[MAX_CLIENT_CONNEXIONS];
	struct pollfd pfds[MAX_CLIENT_CONNEXIONS];
	nfds_t nb_clients = 0;
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

    //on ajoute server_fd au tableau pollfd requis pour poll
	add_descriptor_to_poll(server_fd, Clients, &nb_clients);
	while (1)
	{
		if ((nb_ready_clients = poll((struct pollfd *)Clients, nb_clients, -1)) == -1)
		{
			std::cout << "Client monitoring error " << errno << " -> poll() : " << strerror(errno) << std::endl;
			break ;
		}
		for (nfds_t i = 0; i < nb_clients; i++)
		{
			if (Clients[i].revents & POLLIN)
			{
				// 0 étant l'index dans le tableau pfds pour server_fd
				if (i == 0)
				{
						std::cout << "ALLO" << std::endl;
                    // fcntl O_NONBLOCK du coup la fonction se bloque pas si elle a pas de nouvelles connexions
					if ((client_fd = accept(server_fd, NULL, 0)) == -1)
					{
						std::cout << "Client monitoring error " << errno << " -> accept() : " << strerror(errno) << std::endl;
						break ;
					}
					add_descriptor_to_poll(client_fd, Clients, &nb_clients);
				}
				else
				{
					recv_length = recv(Clients[i].fd, recv_buf, RECV_BUF_SIZE, 0); 
					if (recv_length < 0)
					{
						if (recv_length == -1)
							std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						break ;
					}
					else if (recv_length == 0)
					{
						std::cout << "Connexion stopped with client_fd=" << client_fd << std::endl;
						close(Clients[i].fd);
						remove_descriptor_from_poll(Clients[i].fd, Clients, &nb_clients);
					}
					else 
					{
						recv_buf[recv_length] = 0;
						std::cout << "RECVEID" << std::endl;
						//parse_client_packet(Clients[i], recv_buf);
						// send_msg_to_all_fds(pfds, recv_buf, recv_length,
						// nb_pfds, pfds[i].fd);
					}
				}
			}
		}
	}
	return (ERROR);
}

int main(int argc, char **argv)
{
	int server_fd;

	setbuf(stdout, NULL); // debug, pour éviter que printf ou cout print les lignes que quand y'a un /n, à la place il print à chaque caractère
	(void)argc;
	(void)argv;

	// if (argc == 3)
	// {
		if ((server_fd = create_server_descriptor()) >= 3)
		{
			monitor_clients(server_fd);
		}
	// }

    return 0;
}
