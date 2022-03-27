#ifndef CHANNELSLIST_HPP
# define CHANNELSLIST_HPP

#include "Server.hpp"

#define MAX_ALLOWED_CHANNELS_PER_CLIENT 10
#define MAX_ALLOWED_CHANNELS (MAX_ALLOWED_CLIENTS - 1) * MAX_ALLOWED_CHANNELS_PER_CLIENT //-1 car Clients[0] n'est pas utilisé

class ChannelsList
{
    public:
        std::string name;
		std::string password;
        std::string mode;
		std::vector<int> users;
		std::vector<int> operators;
};

#endif