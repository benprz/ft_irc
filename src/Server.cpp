#include "Server.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
	_server_fd = create_server_fd();
	nfds = 0;
	nchannels = 0;
	bzero(_Clients, sizeof(_Clients));
	bzero(_Channels, sizeof(_Channels));
}

Server::~Server()
{
	close(_server_fd);
}

void	Server::printpfds() // debug
{
	nfds_t npfds2 = nfds;

	std::cout << "\n#-------- pfds list ---------#\n";
	std::cout << "pfds[" << 0 << "]" << std::endl;
	std::cout << "	fd=" << pfds[0].fd << std::endl;
	std::cout << "	events=" << pfds[0].events << std::endl;
	std::cout << "	revents=" << pfds[0].revents << std::endl;
	for (int i = 1; i < npfds2; i++)
	{
		if (pfds[i].fd == -1)
		{
			npfds2++;
			continue ;
		}
		std::cout << std::endl << "pfds[" << i << "]" << std::endl;
		std::cout << "	fd=" << pfds[i].fd << std::endl;
		std::cout << "	events=" << pfds[i].events << std::endl;
		std::cout << "	revents=" << pfds[i].revents << std::endl;
	}
	std::cout << "#----------------------------#\n\n";
}

int Server::create_server_fd(void) const
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
	server_addr.sin_port = htons(_port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

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
	if (listen(server_fd, MAX_ALLOWED_CLIENTS) == -1)
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
void    Server::add_client(int fd)
{
	nfds_t i = 0;

	while (i < nfds)
	{
		if (pfds[i].fd == -1)
			break ;
		i++;
	}
	std::cout << "Added fd=" << fd << " to pfds[" << i << "] with an actual nbpfds=" << nfds + 1 << std::endl;
	if (i > 0)
	{
		bzero(&_Clients[i], sizeof(ClientsMonitoringList));
		_Clients[i].fd = fd;
	}
    pfds[i].fd = fd;
    pfds[i].events = POLLIN;
    nfds++;
	printpfds();
}


void	Server::remove_client_from_all_chans()
{
	for (int i = 0; i < nchannels; i++)
		_Channels[i].remove_user(current_pfd);
}

void    Server::remove_client()
{
	remove_client_from_all_chans();
	close(pfds[current_pfd].fd);
	_Clients[current_pfd].fd = -1;
	pfds[current_pfd].fd = -1;
	pfds[current_pfd].events = 0;
	pfds[current_pfd].revents = 0;
	nfds--;
	printpfds();
}

void Server::launch(void)
{
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

    //on ajoute server_fd au tableau pollfd requis pour poll
	add_client(_server_fd);
	while (1)
	{
		if ((nb_ready_clients = poll(pfds, nfds, -1)) == -1)
		{
			std::cout << "Client monitoring error " << errno << " -> poll() : " << strerror(errno) << std::endl;
			break ;
		}
		for (current_pfd = 0; current_pfd < nfds; current_pfd++)
		{
			if (pfds[current_pfd].revents & POLLIN)
			{
				// 0 étant l'index dans le tableau pfds pour server_fd
				if (current_pfd == 0)
				{
                    // fcntl O_NONBLOCK du coup la fonction se bloque pas si elle a pas de nouvelles connexions
					if ((client_fd = accept(_server_fd, NULL, 0)) == -1)
					{
						std::cout << "Client monitoring error " << errno << " -> accept() : " << strerror(errno) << std::endl;
						break ;
					}
					add_client(client_fd);
				}
				else
				{
					recv_length = recv(pfds[current_pfd].fd, recv_buf, RECV_BUF_SIZE, 0); 
					//si y'a une erreur
					if (recv_length < 0)
					{
						std::cout << "Error connexion stopped with client fd=" << client_fd <<  " events=" << pfds[current_pfd].events << " revents=" << pfds[current_pfd].revents << std::endl;
						std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						remove_client();
						continue ;
					}
					//la connexion s'est coupée, EOF, on supprime donc le fd
					else if (recv_length == 0)
					{
						std::cout << "Connexion stopped with client_fd=" << client_fd << std::endl;
						remove_client();
					}
					//on a reçu un paquet! on l'ouvre :-)
					else 
					{
						recv_buf[recv_length] = 0;
						parse_client_packet(recv_buf);
					}
				}
			}
		}
	}
}
