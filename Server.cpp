#include "Server.hpp"
#include "ClientsMonitoringList.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
}

Server::~Server()
{
}

void	print_pfds(struct pollfd *pfds, nfds_t npfds) // debug
{
	std::cout << "\n#-------- pfds list ---------#\n";
	std::cout << "pfds[" << 0 << "]" << std::endl;
	std::cout << "	fd=" << pfds[0].fd << std::endl;
	std::cout << "	events=" << pfds[0].events << std::endl;
	std::cout << "	revents=" << pfds[0].revents << std::endl;
	for (int i = 1; i < npfds; i++)
	{
		if (pfds[i].fd == -1)
		{
			npfds++;
			continue ;
		}
		std::cout << std::endl << "pfds[" << i << "]" << std::endl;
		std::cout << "	fd=" << pfds[i].fd << std::endl;
		std::cout << "	events=" << pfds[i].events << std::endl;
		std::cout << "	revents=" << pfds[i].revents << std::endl;
	}
	std::cout << "#----------------------------#\n\n";
}

int Server::create_server_descriptor(void) const
{
	int server_fd, sockopt_reuseaddr_val;
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
void    Server::add_descriptor_to_poll(int fd, ClientsMonitoringList *Clients, struct pollfd *pfds, nfds_t &nb_pfds)
{
	nfds_t i = 0;

	while (i < nb_pfds)
	{
		if (pfds[i].fd == -1)
			break ;
		i++;
	}
	std::cout << "Added fd=" << fd << " to pfds[" << i << "] with an actual nb_pfds=" << nb_pfds + 1 << std::endl;
	if (i > 0)
	{
		bzero(&Clients[i], sizeof(ClientsMonitoringList));
		Clients[i].setFd(fd);
		Clients[i - 1].next = &Clients[i];
		if (i > 1)
			Clients[i].prev = &Clients[i - 1];
	}
    pfds[i].fd = fd;
    pfds[i].events = POLLIN;
    nb_pfds++;
}

void    Server::remove_descriptor_from_poll(ClientsMonitoringList &Client, struct pollfd &pfd, nfds_t &nb_pfds)
{
	Client.setFd(-1);
	pfd.fd = -1;
	pfd.events = 0;
	pfd.revents = 0;
	nb_pfds--;
}

int		Server::monitor_clients(int server_fd)
{
	struct pollfd pfds[MAX_CLIENT_CONNEXIONS];
	ClientsMonitoringList Clients[MAX_CLIENT_CONNEXIONS];
	nfds_t nb_clients = 0;
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

	bzero(Clients, sizeof(Clients));
    //on ajoute server_fd au tableau pollfd requis pour poll
	add_descriptor_to_poll(server_fd, NULL, pfds, nb_clients);
	print_pfds(pfds, nb_clients);
	while (1)
	{
		if ((nb_ready_clients = poll(pfds, nb_clients, -1)) == -1)
		{
			std::cout << "Client monitoring error " << errno << " -> poll() : " << strerror(errno) << std::endl;
			break ;
		}
		for (nfds_t i = 0; i < nb_clients; i++)
		{
			if (pfds[i].revents & POLLIN)
			{
				// 0 étant l'index dans le tableau pfds pour server_fd
				if (i == 0)
				{
                    // fcntl O_NONBLOCK du coup la fonction se bloque pas si elle a pas de nouvelles connexions
					if ((client_fd = accept(server_fd, NULL, 0)) == -1)
					{
						std::cout << "Client monitoring error " << errno << " -> accept() : " << strerror(errno) << std::endl;
						break ;
					}
					add_descriptor_to_poll(client_fd, Clients, pfds, nb_clients);
					print_pfds(pfds, nb_clients);
				}
				else
				{
					recv_length = recv(pfds[i].fd, recv_buf, RECV_BUF_SIZE, 0); 
					//si y'a une erreur
					if (recv_length < 0)
					{
						std::cout << "Error connexion stopped with client fd=" << client_fd <<  " events=" << pfds[i].events << " revents=" << pfds[i].revents << std::endl;
						std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						close(pfds[i].fd);
						remove_descriptor_from_poll(Clients[i], pfds[i], nb_clients);
						print_pfds(pfds, nb_clients);
						continue ;
					}
					//la connexion s'est coupée, EOF, on supprime donc le fd
					else if (recv_length == 0)
					{
						std::cout << "Connexion stopped with client_fd=" << client_fd << std::endl;
						close(pfds[i].fd);
						remove_descriptor_from_poll(Clients[i], pfds[i], nb_clients);
						print_pfds(pfds, nb_clients);
					}
					//on a reçu un paquet! on l'ouvre :-)
					else 
					{
						recv_buf[recv_length] = 0;
						Clients[i].parse_client_packet(recv_buf, _password);
					}
				}
			}
		}
	}
	return (ERROR);
}

int Server::launch(void)
{
	int server_fd;

	if ((server_fd = create_server_descriptor()) >= 3)
	{
		if (monitor_clients(server_fd) == 0)
			return (0);
	}
	return (ERROR);
}
