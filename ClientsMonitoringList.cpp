
#include "ClientsMonitoringList.hpp"

void	ClientsMonitoringList::PASS(std::vector<std::string> split_packet)
{
	std::cout << "pass command!" << std::endl;
}

void	ClientsMonitoringList::NICK(std::vector<std::string> split_packet)
{
	std::cout << "nick command!" << std::endl;
}

std::vector<std::string> string_split(std::string s, const char delimiter)
{
    size_t start=0;
    size_t end=s.find_first_of(delimiter);
    
    std::vector<std::string> output;
    
    while (end <= std::string::npos)
    {
	    output.__emplace_back(s.substr(start, end-start));

	    if (end == std::string::npos)
	    	break;

    	start=end+1;
    	end = s.find_first_of(delimiter, start);
    }
    return output;
}


int		ClientsMonitoringList::is_command(std::string command)
{
	for (int i = 0; i < NB_COMMANDS; i++)
	{
		if (command == g_commands_name[i])
			return (1);
	}
	return (-1);
}

void	ClientsMonitoringList::do_command(std::string command, std::vector<std::string> split_packet)
{
	if (command == "PASS")
		PASS(split_packet);
	else if (command == "NICK")
		NICK(split_packet);
}

void	ClientsMonitoringList::parse_client_packet(int client_fd, std::string packet)
{
	std::string current_command;
	int	newline_pos;

	_current_packet += packet;
	while ((newline_pos = _current_packet.find('\n')) != std::string::npos)
	{
		current_command = _current_packet.substr(0, newline_pos);
		_current_packet.erase(0, newline_pos + 1);
		std::vector<std::string> split_packet = string_split(current_command, ' ');
		if (is_command(split_packet[0]) == 1)
			do_command(split_packet[0], split_packet);
		else
			send(client_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}
	/*
int	ClientsMonitoringList::get_command_index(std::string command)
{
	for (int i = 0; i < NB_COMMANDS; i++)
	{
		if (command == g_commands_name[i])
			return (i);
	}
	return (-1);
}

void	ClientsMonitoringList::parse_client_packet(int client_fd, std::string packet)
{
	int	command_index;
	std::string current_command;
	int	newline_pos;

	_current_packet += packet;
	while ((newline_pos = _current_packet.find('\n')) != std::string::npos)
	{
		current_command = _current_packet.substr(0, newline_pos);
		_current_packet.erase(0, newline_pos + 1);
		std::vector<std::string> split_packet = string_split(current_command, ' ');
		if ((command_index = get_command_index(split_packet[0])) >= 0)
		{
			std::cout << "command_index=" << command_index << std::endl;
			(this->*g_commands_functions[command_index])(split_packet);
		}
		else
			send(client_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
	}
}

// Je ferai mieux cette commande une fois que ça sera réglé
void	ClientsMonitoringList::parse_client_packet(int client_fd, std::string packet)
{
	// int	command_index;
	std::string current_command;
	int	newline_pos;

	_current_packet += packet;
	while ((newline_pos = _current_packet.find('\n')) != std::string::npos)
	{
		current_command = _current_packet.substr(0, newline_pos);
		//current_command.erase(current_command.find('\r'));
		//std::cout << "(" << Client.current_packet << ")" << std::endl;
		_current_packet.erase(0, newline_pos + 1);
		std::vector<std::string> split_packet = string_split(current_command, ' ');
		// Je suis pas sûr qu'on ait besoin de check ça
		// if (split_packet.size() > 0)
		// {
			// if ((command_index = get_command_index(split_packet[0])) >= 0)
				// g_commands_functions[command_index](split_packet);
			if (is_command(split_packet[0]) == 1)
				do_command(split_packet[0], split_packet);
			else
				send(client_fd, "Error: unknown command\n", strlen("Error: unknown command\n"), 0);
		// }
		// else
		// 	send(client_fd, "Error: no input command\n", strlen("Error: no input command\n"), 0);
	}
}
*/