#ifndef CHANNELSLIST_HPP
# define CHANNELSLIST_HPP

#include <iostream>
#include <vector>

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

		void	add_user(int client_index);
		void	remove_user(int client_index);
		int		is_invite_only();
		int 	is_user_invited(int client_index);
		int		get_users_number();
		int		is_users_limit_reached();
		void	remove_user_from_invite_list(int client_index);
};

#endif