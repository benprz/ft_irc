// #include "../inc/main.hpp"
#include "../inc/Server.hpp"
#include "../inc/Client.hpp"
#include "../inc/Message.hpp"

#include "../inc/main.hpp"


int main (int ac, char *av[]) {

        if (ac != 3)
        {
                std::cout << "Please provide port and password as follows :\n\t$ ./ircserv <port> <password>" << '\n';
                exit(1);
        }

        /*Still Needs checker for params, only checking param count*/

        std::string temp_pass(av[2]);
        Server serv(atoi(av[1]), temp_pass);

        int ret = serv.launch();

        return(ret);
}
