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
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_index)
			return ;
	}
	if (is_invite_only())
		remove_user_from_invite_list(client_index);
	users.push_back(client_index);
}

void	ChannelsList::remove_user(int client_index)
{
	for (int i = 0; i < users.size(); i++)
	{
		if (users[i] == client_index)
		{
			users.erase(users.begin() + i);
			if (users.size() == 0)
				clear();
		}
	}
}

void	ChannelsList::add_operator(int client_index)
{
	operators.push_back(client_index);
}

void	ChannelsList::remove_operator(int client_index)
{
	for (int i = 0; i < operators.size(); i++)
	{
		if (operators[i] == client_index)
			operators.erase(operators.begin() + i);
	}
}

int		ChannelsList::has_mode(char mode)
{
	if (this->mode.find(mode) != std::string::npos)
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

int	ChannelsList::is_users_limit_reached()
{
	if (users.size() == users_limit)
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

std::string	ChannelsList::add_or_remove_mode(char action, char mode, std::string third_param, Server &serv)
{
	std::cout << "action=" << action << " mode=" << mode << std::endl;

	if (static_cast<std::string>(CHANNEL_MODES).find(mode) == std::string::npos)
		return (ERR_UNKNOWNMODE);
	else
	{
		int mode_pos = this->mode.find(mode);
		int client_index = serv.get_client_id(third_param);

		if (mode == 'o')
		{
				std::cout << "client_index=" << client_index << std::endl;
			if (client_index >= 0)
			{
				if (action == '+')
				{
					if (!is_user_operator(client_index))
						add_operator(client_index);
				}
				else if (is_user_operator(client_index))
					remove_operator(client_index);
			}
		}
		else if (mode == 'l')
		{
			if (action == '+')
			{
				if (!is_limited() && third_param != "")
				{
					for (int i = 0; i < third_param.size(); i++)
					{
						if (!std::isdigit(third_param[i]))
							return "";
					}
					users_limit = std::stoi(third_param);
					this->mode += 'l';
				}
			}
			else if (is_limited())
				this->mode[mode_pos] = ' ';
		}
		else if (mode == 'k')
		{
			if (action == '+')
			{
				if (is_restricted_by_key())
					return (ERR_KEYSET);
				else if (third_param == "")
					return (ERR_NEEDMOREPARAMS);
				else
				{
					this->mode += 'k';
					password = third_param;
				}
			}
			else if (is_restricted_by_key())
				this->mode[mode_pos] = ' ';
		}
		else
		{
			if (action == '+')
			{
				if (!has_mode(mode))
					this->mode += mode;
			}
			else if (has_mode(mode))
				this->mode[mode_pos] = ' ';
		}

	}
	this->mode.erase(std::remove_if(this->mode.begin(), this->mode.end(), ::isspace));
	return ("");
}