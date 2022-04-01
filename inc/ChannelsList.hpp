#ifndef CHANNELSLIST_HPP
# define CHANNELSLIST_HPP

#include "NumericReplies.hpp"

#include <iostream>
#include <vector>

#define CHANNEL_MODES "ositnmlk"

class Server;

class ChannelsList
{
    public:
        std::string name;
		std::string key;
        std::string modes;
		std::vector<int> users;
		std::vector<int> invited_users;
		std::vector<int> operators;
		std::string topic;
		int users_limit;

		ChannelsList(std::string name);
		void	add_user(int client_fd);
		void	remove_user(int client_fd);
		std::string	add_operator(int client_fd);
		std::string	remove_operator(int client_fd);
		int		is_user_on_channel(int client_fd);
		int		is_user_operator(int client_fd);
		int 	is_user_invited(int client_fd);
		int		is_users_limit_reached();
		void	add_user_to_invite_list(int client_fd);
		void	remove_user_from_invite_list(int client_fd);
		void	set_key(std::string key);
		std::string	remove_mode(char mode);
		std::string	add_mode(char mode);
		int		is_mode(char mode);
		int		has_mode(char mode);

		int is_secret() { return (has_mode('s')); };
		int is_invite_only() { return (has_mode('i')); };
		int is_restricted_to_outsiders() { return (has_mode('n')); };
		int is_moderated() { return (has_mode('m')); };
		int is_limited() { return (has_mode('l')); };
		int is_restricted_by_key() { return (has_mode('k')); };
		int is_topic_moderated() { return (has_mode('t')); };
};

#include "Server.hpp"

#endif