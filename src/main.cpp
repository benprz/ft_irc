#include "../inc/Server.hpp"

int main(int argc, char **argv)
{
	setbuf(stdout, NULL); // debug, pour éviter que printf ou cout print les lignes que quand y'a un /n, à la place il print à chaque caractère

	if (argc == 3)
	{
		Server sylbenico(atoi(argv[1]), argv[2]);
		sylbenico.launch();
	}
	else
		std::cout << "Usage : ./ircserv <port> <password>" << std::endl;
	return (errno);
}
