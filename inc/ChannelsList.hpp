#ifndef CHANNELSLIST_HPP
# define CHANNELSLIST_HPP

#include <iostream>
#include <vector>

//std::string g_channel_modes = "opsitnmlk"; // manque b et v, pas sûr de les rajouter

class ChannelsList
{
    public:
        std::string name;
		std::string password;
        std::string mode;
		std::vector<int> users;
		std::vector<int> invited_users;
		std::vector<int> operators;
		int users_limit;

		void	clear();
		void	add_user(int client_index);
		void	remove_user(int client_index);
		void	add_operator(int client_index);
		int		is_invite_only();
		int 	is_user_invited(int client_index);
		int		is_user_on_channel(int client_index);
		int		is_user_operator(int client_index);
		int		get_users_number();
		int		get_operators_number();
		int		is_users_limit_reached();
		void	remove_user_from_invite_list(int client_index);
		//std::string	add_or_remove_mode(char action, char mode, std::string third_param, Server &serv);
};

#endif