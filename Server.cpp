#include "Server.hpp"

Server::Server() : _port(666), _password("dumbpassword")
{
	return;
}

Server::Server(int const port, std::string const password) : _port(port), _password(password)
{
	// std::cout << "Intance's port = " << getPort() << " & password = " << getPassword() << std::endl; // del
}

Server::~Server()
{
	return;
}

// int Server::getPort() const
// {
// 	return (this->_port);
// }

// std::string Server::getPassword() const
// {
// 	return (this->_password);
// }

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

void	Server::add_descriptor_to_poll(int fd, struct pollfd *pfds, nfds_t *nb_pfds)
{
    pfds[*nb_pfds].fd = fd;
    pfds[*nb_pfds].events = POLLIN;
    pfds[*nb_pfds].revents = 0;
    *nb_pfds += 1;
}

int		Server::monitor_clients(int server_fd)
{
	// en c++ une classe, c'est exactement comme une structure presque (vérifie sur internet tu verras), et le truc
	// c'est que ClientsMonitoringList hérite de pollfd donc c'est comme si c'était pollfd. Et ça marche.
	// Sauf que poll arrive pas à gérer ça, je comprends pas pq il arrive juste pas à changer la valeurs des variables revents
	// pour chaque client, et du coup bah le programme marche que si tu laisses Clients avec le type pollfd au lieu de
	// ClientsMonitoringList. Je suis un peu deg :/ Si tu veux chercher une autre méthode de structurer tout ça pour qu'on puisse
	// tout regrouper par client, ça serait cool. 
	// Je te laisse la fonction monitor_clients comme ça, c'est juste une version simplifiée sans les messages d'erreurs et sans
	// le recv du code en commentaire juste en dessous. Comme ça tu pourras expérimenter des trucs si tu veux pour mieux
	// comprendre comment la boucle de monitoring fonctionne (poll, accept, recv)

	// ClientsMonitoringList Clients[MAX_CLIENT_CONNEXIONS];
	struct pollfd Clients[MAX_CLIENT_CONNEXIONS];
	nfds_t nb_clients = 0;

	add_descriptor_to_poll(server_fd, Clients, &nb_clients);
	while (1)
	{
		int nb_ready_clients = poll(static_cast<struct pollfd *>(Clients), nb_clients, -1);
		for (int i = 0; i < nb_clients; i++)
		{
			if (Clients[i].revents & POLLIN)
			{
				if (i == 0)
				{
					int client_fd = accept(server_fd, NULL, NULL);
					add_descriptor_to_poll(client_fd, Clients, &nb_clients);
				}
				else
				{
					std::cout << "Message reçu, et j'exit() parce que je sais pas quoi faire!" << std::endl;
					exit(0);
				}
			}
		}
	}
	/*
	// struct pollfd	Clients[MAX_CLIENT_CONNEXIONS];
	ClientsMonitoringList	Clients[MAX_CLIENT_CONNEXIONS];
	nfds_t nb_clients = 0;
    int nb_ready_clients, client_fd;
	char	recv_buf[RECV_BUF_SIZE + 1];
	ssize_t	recv_length;

    //on ajoute server_fd au tableau pollfd requis pour poll
	add_descriptor_to_poll(server_fd, Clients, &nb_clients);
	while (1)
	{
		if ((nb_ready_clients = poll(static_cast<struct pollfd *>(Clients), nb_clients, -1)) == -1)
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
					//si y'a une erreur
					if (recv_length < 0)
					{
						if (recv_length == -1)
							std::cout << "Client monitoring error " << errno << " -> recv() : " << strerror(errno) << std::endl;
						break ;
					}
					//la connexion s'est coupée, EOF, on supprime donc le fd
					else if (recv_length == 0)
					{
						std::cout << "Connexion stopped with client_fd=" << client_fd << std::endl;
						close(Clients[i].fd);
						remove_descriptor_from_poll(Clients[i].fd, Clients, &nb_clients);
					}
					//on a reçu un paquet! on l'ouvre :-)
					else 
					{
						std::cout << "RCVEVEVEVE" << std::endl;
						recv_buf[recv_length] = 0;
						parse_client_packet(Clients[i], recv_buf);
						// send_msg_to_all_fds(pfds, recv_buf, recv_length, nb_pfds, pfds[i].fd);
					}
				}
			}
		}
	}
	return (ERROR);
	*/
}

int Server::launch(void)
{
	int server_fd;

	if ((server_fd = create_server_descriptor()) >= 3)
	{
		monitor_clients(server_fd);
	}
	return (Server::create_server_descriptor());
}