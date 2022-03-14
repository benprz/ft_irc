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

#define ERROR 1 // je trouve -1 plus logique perso
#define PROTO "TCP"
#define PORT 16385
#define SOCK_DOMAIN AF_INET
#define IP_ADDR "127.0.0.1"
#define SOCK_TYPE SOCK_STREAM
#define RECV_BUF_SIZE 1024
#define MAX_CLIENT_CONNEXIONS 1
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
void	add_descriptor_to_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
	pfds[*nb_pfds].fd = fd;
	pfds[*nb_pfds].events = POLLIN;
	pfds[*nb_pfds].revents = 0;
	*nb_pfds += 1;
}

void	remove_descriptor_from_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
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

void	send_msg_to_all_fds(struct pollfd *pfds, char *buf, ssize_t length, nfds_t nb_pfds, int sender_fd)
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

void	parse_client_packets(int client_fd, std::string packets)
{
	std::vector<std::string> split_packets = string_split(packets, ' ');

	std::string command = split_packets[0];
	std::string param = split_packets[1];

	std::cout << packets;
	if (command.compare("PASS") == 0)
	{
		param.pop_back();
		param.pop_back();
		if (param.compare(PASSWORD) != 0)
		{
			uint32_t error = ERR_PASSWDMISMATCH;
			std::cout << "Erreur, mot de passe incorrect" << std::endl;
			send(client_fd, "Erreur: incorrect password", strlen("Erreur: incorrect password"), 0);
		}
	}
}

int main(void)
{
    int server_fd, nb_ready_clients, client_fd;
    struct pollfd pfds[MAX_CLIENT_CONNEXIONS];
    nfds_t nb_pfds = 0;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

    setbuf(stdout, NULL); // debug, pour éviter que printf print les lignes que
						  // quand y'a un \n, à la place il print à chaque
						  // caractère
	server_fd = create_server_descriptor();

    //on ajoute server_fd au tableau pollfd requis pour poll
	add_descriptor_to_poll(server_fd, pfds, &nb_pfds);

	while (1)
	{
		if ((nb_ready_clients = poll(pfds, nb_pfds, -1)) == -1)
		{
			std::cout << "Client socket error " << errno << " -> poll() : " << strerror(errno) << std::endl;
			return (ERROR);
		}
		for (int i = 0; i < nb_pfds; i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				// 0 étant l'index dans le tableau pfds pour server_fd
				if (i == 0)
				{
                    // fcntl O_NONBLOCK du coup la fonction se bloque pas si
					// elle a pas de nouvelles connexions
					if ((client_fd = accept(server_fd, NULL, 0)) == -1)
					{
						std::cout << "Client socket error " << errno << " -> accept() : " << strerror(errno) << std::endl;
						return (ERROR);
					}
					add_descriptor_to_poll(client_fd, pfds, &nb_pfds);
				}
				else 
				{
					recv_length = recv(pfds[i].fd, recv_buf, RECV_BUF_SIZE, 0); 
					if (recv_length <= 0)
					{
						if (recv_length == -1)
							std::cout << "Client socket error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						close(pfds[i].fd);
						remove_descriptor_from_poll(pfds[i].fd, pfds, &nb_pfds);
					}
					else 
					{
						recv_buf[recv_length] = 0;
						parse_client_packets(client_fd, recv_buf);
						// send_msg_to_all_fds(pfds, recv_buf, recv_length,
						// nb_pfds, pfds[i].fd);
					}
				}
			}
		}
	}
    return 0;
}
