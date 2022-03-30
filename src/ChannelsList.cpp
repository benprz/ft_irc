#include "ChannelsList.hpp"

void	ChannelsList::clear()
{
	name.clear();
	password.clear();
	mode.clear();
	users.clear();
	invited_users.clear();
	operators.clear();
	users_limit = 0;
}

void	ChannelsList::add_user(int client_index)
{
	if (is_invite_only())
		remove_user_from_invite_list(client_index);
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == -1)
		{
			users[i] = client_index;
			return ;
		}
	}
	users.push_back(client_index);
}

void	ChannelsList::remove_user(int client_index)
{
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_index)
		{
			users[i] = -1;
			if (get_users_number() == 0)
				clear();
			break ;
		}
	}
}

void	ChannelsList::add_operator(int client_index)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_index)
		{
			operators[i] = -1;
			if (get_operators_number() == 0)
				operators.clear();
			break ;
		}
	}
}

int	ChannelsList::is_invite_only()
{
	if (mode.find('i') != std::string::npos)
		return (1);
	return (0);
}

int ChannelsList::is_user_invited(int client_index)
{
	for (int i = 0; i < invited_users.size(); i++)
	{
		if (invited_users[i] == client_index)
			return (1);
	}
	return (0);
}

int	ChannelsList::is_user_on_channel(int client_index)
{
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_index)
			return (1);
	}
	return (0);
}

int	ChannelsList::is_user_operator(int client_index)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_index)
			return (1);
	}
	return (0);
}


int	ChannelsList::get_users_number()
{
	int count = 0;

	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] != -1)
			count++;
	}
	return (count);
}

int	ChannelsList::get_operators_number()
{
	int count = 0;

	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] != -1)
			count++;
	}
	return (count);
}

int	ChannelsList::is_users_limit_reached()
{
	if (get_users_number() == users_limit)
		return (1);
	return (0);
}

void	ChannelsList::remove_user_from_invite_list(int client_index)
{
	for (int i = 0; i < invited_users.size(); i++)
	{
		if (invited_users[i] == client_index)
		{
			invited_users[i] = -1;
			break ;
		}
	}
}

/*
std::string	ChannelsList::add_or_remove_mode(char action, char mode, std::string third_param, Server &serv)
{
	if (g_channel_modes.find(mode) != std::string::npos)
		return (ERR_UNKNOWNMODE);
	else
	{
		int mode_pos = this->mode.find(mode);
		if (mode == 'k')
		{
			if (action == '+')
			{
				if (mode_pos == std::string::npos)
					return (ERR_KEYSET);
				else if (is_user_operator(serv.current_pfd))
				{
					if (third_param == "")
						return (ERR_NEEDMOREPARAMS);
					else
					{
						this->mode += 'k';
						password = third_param;
					}
				}
			}
			else
			{
				if (mode_pos != std::string::npos)
					this->mode[mode_pos] = ' ';
			}
		}
		else if (mode == 'o')
		{
			if (is_user_operator(serv.current_pfd))
			{
				int given_nick_client_index = serv.get_client_id(third_param);
				if (given_nick_client_index >= 0 && is_user_operator(given_nick_client_index))
				{
				//	add_oper_user(given_nick_client_index);
				}
			}
		}
	}
}

std::string	ChannelsList::remove_mode(char mode)
{
	if (g_channel_modes.find(mode) != std::string::npos)
		return (ERR_UNKNOWNMODE);
	else
		this->mode += mode;
}
*/