#ifndef CHANNELSLIST_HPP
# define CHANNELSLIST_HPP

#include "Server.hpp"

#include <iostream>

#define MAX_ALLOWED_CHANNELS MAX_ALLOWED_CLIENTS * 10 // * 10 car le max de channels qu'un utilisateur peut creer (ou joindre) est 10

class ChannelsList
{
    private:
        std::string _name;
        std::string _mode;
        std::string _users[MAX_ALLOWED_CLIENTS];
};

#endif