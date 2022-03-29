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