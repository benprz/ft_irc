#include "../inc/Server.hpp"
// #include "../inc/main.hpp"

int Server::modecmd(Message &msg, int fd) {
        if (msg.getParams().size() == 3) {
                if (chan.find(msg.getParams().front()) != chan.end()) {
                        if (find_Cinchan(fd, chan_oper[msg.getParams().front()]) || _m_fd2client[fd].getOp()) {
                                std::list<std::string> temp = msg.getParams();
                                std::list<std::string>::iterator it_args = temp.begin();
                                it_args++;
                                if (*it_args == "+o") {
                                        std::vector<Client*>::iterator it_opcli = chan_oper[msg.getParams().front()].begin();
                                        for (; it_opcli != chan_oper[msg.getParams().front()].end(); it_opcli++) {
                                                if ((**it_opcli).getNick() == msg.getParams().back()) {
                                                        return (0);
                                                }
                                        }
                                        if (it_opcli == chan_oper[msg.getParams().front()].end()) {
                                                std::map<int, Client>::iterator it_clients = _m_fd2client.begin();
                                                for (; it_clients != _m_fd2client.end(); it_clients++) {
                                                        if (it_clients->second.getNick() == msg.getParams().back()) {
                                                                std::cout << "fd found -> " << it_clients->second.getFd() << std::endl;
                                                                Client *temp = &(_m_fd2client[it_clients->second.getFd()]);
                                                                chan_oper[msg.getParams().front()].push_back(temp);
                                                                sendtoAll(fd, chan[msg.getParams().front()], ":" + _m_fd2client[fd].getCprefix() + " " + msg.getCmd() + " " + msg.getParams().front() + " " + *it_args + " " + msg.getParams().back());
                                                                std::cout << "USER ->" << it_clients->second.getNick() << " Has been OP" << std::endl;
                                                        }
                                                }
                                        }
                                }
                                else if (*it_args == "-o") {
                                        std::vector<Client*>::iterator it_opcli = chan_oper[msg.getParams().front()].begin();
                                        for (; it_opcli != chan_oper[msg.getParams().front()].end(); it_opcli++) {
                                                if ((**it_opcli).getNick() == msg.getParams().back()) {
                                                        break;
                                                }
                                        }
                                        if (it_opcli == chan_oper[msg.getParams().front()].end()) {
                                                send_err(fd, ERR_NOSUCHNICK, " <" + msg.getParams().back() + "> :No such nick\r\n");
                                        }
                                        else {
                                                std::cout << "USER ->" << (**it_opcli).getNick() << " Has been deOP" << std::endl;
                                                chan_oper[msg.getParams().front()].erase(it_opcli);
                                                sendtoAll(fd, chan[msg.getParams().front()], ":" + _m_fd2client[fd].getCprefix() + " " + msg.getCmd() + " " + msg.getParams().front() + " " + *it_args + " " + msg.getParams().back());
                                        }
                                }
                                else {
                                        std::cout << "Bad mode ->" << *it_args << std::endl;
                                }
                        }
                        else {
                                send_err(fd, ERR_NOPRIVILEGES," :Permission Denied- You're not an operator or operator of " + msg.getParams().front() + " channel \r\n");
                        }
                }
                else {
                        send_err(fd, ERR_NOSUCHCHANNEL, "<" + msg.getParams().front() + "> :No such channel\r\n");
                }
        }
        else {
                send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Need 3 paramaters\r\n");
        }
        return (0);
}

int Server::quitcmd(int fd) {
        std::cout << "Closing connection for " << _m_fd2client[fd].getNick() << std::endl;
        remove_Cinchans(fd);
        clearClient(fd);
        close(fd);
        FD_CLR(fd, &read_fd_set);
        for (std::vector<int>::iterator it = _socket_fd.begin(); it != _socket_fd.end(); it++) {
                if (*it == _m_fd2client[fd].getFd()) {
                        _socket_fd.erase(it);
                        break;
                }
        }
        std::cout << "User : " << _m_fd2client[fd].getNick() << " has been KILL" << std::endl;
        _m_fd2client.erase(fd);
        return (0);
}

int Server::squitcmd(Message &msg, int fd) {
        exit(0);
}

int Server::namecmd(Message &msg, int fd) {
        if (msg.getParams().size() == 1) {
                std::list<std::string> receiver_list = split_every_char(msg.getParams().front(), ',');
                std::list<std::string>::iterator it_recv = receiver_list.begin();
                for (; it_recv != receiver_list.end(); it_recv++) {
                        dispMemberName(fd, *it_recv);
                }
        }
        return (0);
}

int Server::listcmd(Message &msg, int fd) {
        if (msg.getParams().size() == 0) {
                std::string list_tosend = " ";
                if (chan.size() >= 1) {
                        std::map<std::string, std::vector<Client*> >::iterator it_chan = chan.begin();
                        for (; it_chan != chan.end(); it_chan++) {
                                list_tosend.append(it_chan->first + ",");
                        }
                }
                send_privmsg(fd, "Channel :\n" + list_tosend + "\n" + ":End of /LIST\r\n");
        }
        return (0);
}

int Server::kickcmd(Message &msg, int fd) {
        if (msg.getParams().size() == 3) {
                std::list<std::string> args = msg.getParams();
                std::list<std::string>::iterator it_args;
                it_args = args.begin();
                std::map<std::string, std::vector<Client*> >::iterator it_chan = chan.begin();
                for (; it_chan != chan.end(); it_chan++) {
                        if (it_chan->first == msg.getParams().front())
                                break;
                }
                if (it_chan != chan.end()) {
                        if (find_Cinchan(fd, chan_oper.find(it_chan->first)->second) == true) {
                                it_args++;
                                std::vector<Client*>::iterator it_cli = it_chan->second.begin();
                                for (; it_cli != it_chan->second.end(); it_cli++) {
                                        if ((**it_cli).getNick() == *it_args) {
                                                std::cout << "USER FOUND Let's kick him -->" << (**it_cli).getNick() << std::endl;
                                                sendtoAll(fd, it_chan->second, ":" + _m_fd2client[fd].getCprefix() + " KICK " + it_chan->first + " " + (**it_cli).getNick() + " :" + msg.getParams().back());
                                                break;
                                        }
                                }
                                if (it_cli != it_chan->second.end()) {
                                        it_chan->second.erase(it_cli);
                                }
                                else {
                                        send_err(fd, ERR_NOSUCHNICK, " <" + *it_args + "> :No such nick in the channel " + it_chan->first + "\r\n");
                                }
                        }
                        else {
                                send_err(fd, ERR_NOPRIVILEGES," :Permission Denied- You're not an operator of " + it_chan->first + " channel \r\n");
                        }
                }
                else {
                        send_err(fd, ERR_NOSUCHCHANNEL, "<" + msg.getParams().front() + "> :No such channel\r\n");
                }
        }
        else {
                send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Need 3 paramaters\r\n");
        }
        return (0);
}

int Server::partcmd(Message &msg, int fd) {
        if ((msg.getParams().size() <= 2) && (msg.getParams().size() != 0)) {
                std::string msg_exit = msg.getParams().size() == 2 ? msg.getParams().back() : "";
                std::list<std::string> chan_list = split_every_char(msg.getParams().front(), ',');
                std::list<std::string>::iterator it_chan_list = chan_list.begin();
                while(it_chan_list != chan_list.end())
                {
                        std::map<std::string, std::vector<Client*> >::iterator it_chan = chan.begin();
                        std::string chan_name = *it_chan_list;
                        for (; it_chan != chan.end(); it_chan++) {
                                if (it_chan->first == *it_chan_list)
                                        break;
                        }
                        if (it_chan != chan.end()) {
                                std::vector<Client*>::iterator it_cli = it_chan->second.begin();
                                for (; it_cli != it_chan->second.end(); it_cli++) {
                                        if ((**it_cli).getNick() == _m_fd2client[fd].getNick()) {
                                                sendtoAll(fd, it_chan->second, ":" + _m_fd2client[fd].getCprefix() + " PART " + it_chan->first + " " + (**it_cli).getNick() + " :" + msg_exit);
                                                break;
                                        }
                                }
                                if (it_cli != it_chan->second.end()) {
                                        it_chan->second.erase(it_cli);
                                }
                                else {
                                        send_err(fd, ERR_NOSUCHNICK, " <" + _m_fd2client[fd].getNick() + "> :No such nick in the channel " + it_chan->first + "\r\n");
                                }
                        }
                        else {
                                send_err(fd, ERR_NOSUCHCHANNEL, "<" + msg.getParams().front() + "> :No such channel\r\n");
                        }
                        it_chan_list++;
                }

        }
        else {
                send_err(fd, ERR_NEEDMOREPARAMS, ":Syntax error\r\n");
        }
        return(0);
}


int Server::killcmd(Message &msg, int fd) {
        if (_m_fd2client[fd].getOp() == true) {
                if (msg.getParams().size() != 2) {
                        if (msg.getParams().size() > 2)
                                send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Need 2 paramaters\r\n");
                        else
                                send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Not enough parameters, need 2 paramaters\r\n");
                }
                else if (nick_check(msg.getParams().front(), fd) == true) {
                        send_err(fd, ERR_NOSUCHNICK, " <" + msg.getParams().front() + "> :No such nick\r\n");
                }
                else {
                        std::cout << "searching for clients" << '\n';
                        std::map<int, Client>::iterator it_clients = _m_fd2client.begin();
                        for (; it_clients != _m_fd2client.end(); it_clients++) {
                                int check = it_clients->second.getFd();
                                if (it_clients->second.getNick() == msg.getParams().front() && it_clients->second.getFd() > 0) {
                                        std::cout << "Closing connection for " << it_clients->second.getNick() << std::endl;
                                        send_privmsg(it_clients->second.getFd(), "you got killed : " + msg.getParams().back() + "\r\n");
                                        remove_Cinchans(it_clients->second.getFd());
                                        clearClient(it_clients->second.getFd());
                                        close(it_clients->second.getFd());
                                        FD_CLR(it_clients->second.getFd(), &read_fd_set);
                                        for (std::vector<int>::iterator it = _socket_fd.begin(); it != _socket_fd.end(); it++) {
                                                if (*it == it_clients->second.getFd()) {
                                                        _socket_fd.erase(it);
                                                        break;
                                                }
                                        }
                                        std::cout << "User : " << it_clients->second.getNick() << " has been KILL" << std::endl;
                                        _m_fd2client.erase(it_clients);
                                        break;
                                }
                        }
                }
        }
        else {
                send_err(fd, ERR_NOPRIVILEGES," :Permission Denied- You're not an IRC operator\r\n");
        }
        return (0);
}

int Server::opercmd(Message &msg, int fd) {
        if (msg.getParams().size() != 2) {
                if (msg.getParams().size() > 2)
                        send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Need 2 paramaters\r\n");
                else
                        send_err(fd, ERR_NEEDMOREPARAMS, " <" + msg.getCmd() + "> :Not enough parameters, need 2 paramaters\r\n");
        }
        else if (nick_check(msg.getParams().front(), fd) == true) {
                send_err(fd, ERR_NOSUCHNICK, " <" + msg.getParams().front() + "> :No such nick\r\n");
        }
        else if (msg.getParams().back() == OPERATOR_PW) {
                std::cout << "searching for clients" << '\n';
                std::map<int, Client>::iterator it_clients = _m_fd2client.begin();
                for (; it_clients != _m_fd2client.end(); it_clients++) {
                        if (it_clients->second.getNick() == msg.getParams().front() && it_clients->second.getFd() > 0) {
                                it_clients->second.setOp(true);
                                std::cout << "User : " << it_clients->second.getNick() << " has been OP" << std::endl;
                        }
                }
        }
        else {
                send_err(fd, ERR_PASSWDMISMATCH, " :Password incorrect\r\n");
        }
        return (0);

}

int Server::send_privmsg(int fd, std::string str) {
        return (send(fd, str.c_str(), str.size(), 0));
}

bool Server::nick_check(std::string &nick, int fd) {
        std::map<int, Client>::iterator it = _m_fd2client.begin();
        while(it != _m_fd2client.end()) {
                std::cout << (it->second.getNick()) << '\n';
                if (it->second.getNick() == nick && it->second.getFd() > 0) {
                        return false;
                }
                it++;
        }
        return true;
}


int Server::nickcmd(Message msg, int fd){
        if (msg.getParams().size() == 0) {
                send_err(fd, ERR_NONICKNAMEGIVEN, " :No nickname given\r\n");
        }
        else if (msg.getParams().size() > 1) {
                send_err(fd, ERR_NEEDMOREPARAMS, msg.getCmd() + " :Syntax error\r\n");
        }
        else if (msg.getParams().front() == _m_fd2client[fd].getNick()) {
                send_err(fd, ERR_NICKNAMEINUSE, " <" + msg.getParams().front() +"> :Nickname is already in use\r\n");
        }
        else if (nick_check(msg.getParams().front(), fd) == false) {
                send_err(fd, ERR_NICKCOLLISION, " <" + msg.getParams().front() + "> :Nickname collision\r\n");
        }
        else if (_m_fd2client[fd].getReg()) {
                noticeNickInChans(fd, _m_fd2client[fd].getCprefix(), msg.getParams().front());
                _m_fd2client[fd].setNick(msg.getParams().front());
                std::string prefix;
                prefix = _m_fd2client[fd].getNick();
                prefix += "!";
                prefix += _m_fd2client[fd].getReal();
                std::cout << _m_fd2client[fd].getReal() << '\n';
                prefix += "@";
                prefix += inet_ntoa(_v_clients.back().getInfo().sin_addr);
                _m_fd2client[fd].setCprefix(prefix);
        }
        else {
                _m_fd2client[fd].setNick(msg.getParams().front());
                _m_fd2client[fd].setNickstatus(true);
        }
        if (_m_fd2client[fd].getNickstatus() && _m_fd2client[fd].getUserstatus()) {
                _m_fd2client[fd].setReg(true);
        }
        return(0);
}

Client Server::find_CfromFd(int fd){
        std::vector<Client>::iterator it = _v_clients.begin();
        for (; it != _v_clients.end(); it++) {
                if (it->getFd() == fd) {
                        return(*it);
                }
        }
        return(_v_clients.front());
}

int Server::passcmd(Message & msg, int fd) {

        if (_m_fd2client[fd].getCorr() == false) {
                if (msg.getParams().size() == 0) {
                        send_err(fd, ERR_NEEDMOREPARAMS, " PASS :Not enough parameters\r\n");
                }
                else if (msg.getParams().front() == _password) {
                        _m_fd2client.insert(std::pair<int, Client>(fd, find_CfromFd(fd)));
                        _m_fd2client[fd].setCorr(true);
                }
                else {
                        send_privmsg(fd, "ERROR :Access denied: Bad password?\r\n"); // erreur
                        std::cout << "bad pass" << '\n';
                }
        }
        else {
                send_err(fd, ERR_ALREADYREGISTERED, " :You may not reregister\r\n");
        }
        return(0);
}

void Server::pingcmd(Message & msg, int fd) {
        send_privmsg(fd, "PONG " + msg.getParams().front() + msg.getParams().back() + "\r\n");
}

int Server::usercmd(Message &msg, int fd) {

        //if () user deja register -- >ERR_ALREADYREGISTERED // wrong parameters // pass
        if (_m_fd2client[fd].getNickstatus() == true) {
                if (msg.getParams().size() == 4) {
                        if (_m_fd2client[fd].getReg() == false) {
                                if (_m_fd2client[fd].getReg()) {
                                        std::cout << "erreur" << '\n';
                                }
                                // nb params
                                _m_fd2client[fd].setUser(msg.getParams().front());
                                _m_fd2client[fd].setReal(msg.getParams().back());
                                std::string prefix;


                                prefix = _m_fd2client[fd].getNick();
                                prefix += "!";
                                prefix += _m_fd2client[fd].getReal();
                                std::cout << _m_fd2client[fd].getReal() << '\n';
                                prefix += "@";
                                prefix += inet_ntoa(_v_clients.back().getInfo().sin_addr);
                                std::cout << prefix << '\n';
                                _m_fd2client[fd].setCprefix(prefix);
                                _m_fd2client[fd].setUserstatus(true);
                                _m_fd2client[fd].setFd(fd);
                                if (_m_fd2client[fd].getNickstatus() && _m_fd2client[fd].getUserstatus()) {
                                        _m_fd2client[fd].setReg(true);
                                }
                                send_privmsg(fd, ":irc.example.net 001 " + _m_fd2client[fd].getNick() + " :Welcome to the Internet Relay Network " + _m_fd2client[fd].getCprefix() + "\r\n");
                        }
                        else{
                                send_err(fd, ERR_ALREADYREGISTERED, " :You may not reregister\r\n");
                        }
                }
                else {
                        send_err(fd, ERR_NEEDMOREPARAMS, " <USER> :Not enough parameters\r\n");
                }
        }
        return(0);
}

void Server::noticeNickInChans(int fd, std::string old_pref, std::string new_nick)
{
        std::map<std::string, std::vector<Client*> >::iterator it_chan = chan.begin();
        for (; it_chan != chan.end(); it_chan++) {
                std::vector<Client*>::iterator it_cli = it_chan->second.begin();
                for (; it_cli != it_chan->second.end(); it_cli++) {
                        if ((**it_cli).getFd() == _m_fd2client[fd].getFd()) {
                                sendtoAll(fd, (it_chan)->second, ":" + old_pref + " NICK :" + new_nick);
                        }
                }
        }
}

bool Server::find_Cinchan(int fd, std::vector<Client*> vect)
{
        std::vector<Client*>::iterator it = vect.begin();
        for(; it != vect.end(); it++) {
                if((**it).getNick() == _m_fd2client[fd].getNick())
                        return(true);
        }
        return(false);
}

void Server::remove_Cinchans(int fd)
{
        std::map<std::string, std::vector<Client*> >::iterator it_chan = chan.begin();
        for (; it_chan != chan.end(); it_chan++) {
                std::vector<Client*>::iterator it_cli = it_chan->second.begin();
                for (; it_cli != it_chan->second.end(); it_cli++) {
                        if ((**it_cli).getFd() == _m_fd2client[fd].getFd()) {
                                sendtoAll(fd, (it_chan)->second, ":" + (**it_cli).getCprefix() + " QUIT :Client closed connection");
                                std::cout << "USER FOUND Let's kick him -->" << (**it_cli).getNick() << std::endl;
                                break;
                        }
                }
                if (it_cli != it_chan->second.end()) {
                        if (find_Cinchan(fd, chan_oper[it_chan->first]) == true) {
                                std::vector<Client*>::iterator it_opcli = chan_oper[it_chan->first].begin();
                                for (; it_opcli != chan_oper[it_chan->first].end(); it_opcli++) {
                                        if ((**it_opcli).getFd() == _m_fd2client[fd].getFd()) {
                                                std::cout << "OP USER FOUND Let's kick him -->" << (**it_cli).getNick() << std::endl;
                                                break;
                                        }
                                }
                                if (it_opcli != chan_oper[it_chan->first].end()) {
                                        chan_oper[it_chan->first].erase(it_opcli);
                                }
                        }
                        it_chan->second.erase(it_cli);
                }
        }
}

void Server::sendtoAllbutme(int fd, std::vector<Client*> at, std::string msg)
{
        std::vector<Client*>::iterator client = at.begin();
        for (; client != at.end(); client++) {
                // Client to_send = *(*client);
                int fd_tmp = (*(*client)).getFd();
                if (fd_tmp > 0 && fd != fd_tmp)
                        send_privmsg(fd_tmp, msg + "\r\n");
        }

}

void Server::sendtoAll(int fd, std::vector<Client*> at, std::string msg)
{
        std::vector<Client*>::iterator client = at.begin();
        std::cout << "Sending to everyone : " << "\'" << msg << "\'" << '\n';
        for (; client != at.end(); client++) {
                // Client to_send = *(*client);
                int fd_tmp = (*(*client)).getFd();
                if (fd_tmp > 0) {
                        std::cout << (*(*client)).getNick() << "\t" << (*(*client)).getFd() << '\n';
                        send_privmsg(fd_tmp, msg + "\r\n");
                }
        }

}

std::string Server::getmsg(Message &msg, int fd, std::string name)
{
        std::list<std::string> params = msg.getParams();
        std::string msg_tosend;
        msg_tosend = ":" + _m_fd2client[fd].getCprefix() + " PRIVMSG " + name + " : ";
        std::list<std::string>::iterator it = params.begin();
        it++;
        if (params.size() == 2) {
                msg_tosend.append(*it);
        }
        else {
                send_err(fd, ERR_NEEDMOREPARAMS, "Syntax Error\r\n"); // changer !
        }
        return (msg_tosend);
}

void Server::privmsg(Message &msg, int fd)
{
        if (msg.getParams().size() >= 2) {
                std::cout << "privmsg" << std::endl;
                std::list<std::string> receiver_list = split_every_char(msg.getParams().front(), ',');
                std::list<std::string>::iterator it_recv = receiver_list.begin();
                while(it_recv != receiver_list.end())
                {
                        std::string msg_tosend = getmsg(msg, fd, *it_recv);
                        std::cout << "it_recv -> " << *it_recv << std::endl;
                        if ((*it_recv)[0] == '#') {
                                if (chan.find(*it_recv) != chan.end()) {
                                        sendtoAllbutme(fd, chan[*it_recv], msg_tosend);
                                }
                                else {
                                        send_err(fd, ERR_CANNOTSENDTOCHAN, "<" + *it_recv + "> :Cannot send to channel\r\n");
                                }
                        }
                        else {
                                std::cout << "searching for clients" << '\n';
                                std::map<int, Client>::iterator it_clients = _m_fd2client.begin();
                                for (; it_clients != _m_fd2client.end(); it_clients++) {
                                        if (it_clients->second.getNick() == *it_recv) {
                                                std::cout << "let's send some shit boy -> " << msg_tosend << std::endl;
                                                std::cout << "fd found -> " << it_clients->second.getFd() << std::endl;
                                                send_privmsg(it_clients->second.getFd(), msg_tosend + "\r\n");
                                        }
                                }
                        }
                        it_recv++;
                }
        }
        else {
                send_err(fd, ERR_NOTEXTTOSEND, " :No text to send     or     ");
                send_err(fd, ERR_NOSUCHNICK, " <" + msg.getParams().front() + "> :No such nick/channel\r\n");
        }
}

void Server::dispMemberName(int fd, std::string chan_name) {
        std::string nick = _m_fd2client[fd].getNick();
        send_privmsg(fd, ( ":" + static_cast<std::string>(SERV_NAME) + " " + static_cast<std::string>(RPL_NAMREPLY) + " " + nick + " = " + chan_name + " :")); // :irc.example.net 353 koko = #chan :@koko
        std::vector<Client *>::iterator it = chan[chan_name].begin();
        for(; it != chan[chan_name].end(); it++) {
                if ((**it).getOp() || find_Cinchan((**it).getFd(), chan_oper[chan_name]) == true) {
                        send_privmsg(fd, "@" + (**it).getNick() + " ");
                }
                else {
                        send_privmsg(fd, (**it).getNick() + " ");
                }
        }
        send_privmsg(fd, "\r\n");
        send_privmsg(fd, ":" + static_cast<std::string>(SERV_NAME) + " " + static_cast<std::string>(RPL_ENDOFNAMES) + " " + nick + " " + chan_name + " :End of NAMES list\r\n"); //:irc.example.net 366 koko #jiji :End of NAMES list
}

int Server::joincmd(Message &msg, int fd)
{
        if (msg.getParams().size() <= 2) {
                std::list<std::string> chan_list = split_every_char(msg.getParams().front(), ',');
                std::list<std::string>::iterator it_chan = chan_list.begin();
                std::list<std::string> pass_list = split_every_char(msg.getParams().back(), ',');
                std::list<std::string>::iterator it_pass = pass_list.begin();
                while((it_chan != chan_list.end()) && (it_pass != pass_list.end()))
                {
                        std::string chan_name = *it_chan;
                        size_t size = msg.getParams().size();
                        if(size <= 2 && size >= 1) {
                                if (chan_name[0] == '#')
                                {
                                        if (chan.find(chan_name) != chan.end())
                                        {
                                                if (find_Cinchan(fd, chan.find(chan_name)->second) != true)
                                                {
                                                        if ((chan_pass.find(*it_chan) == chan_pass.end()) || (chan_pass.find(*it_chan)->second == *it_pass))
                                                        {
                                                                Client *temp = &(_m_fd2client[fd]);
                                                                chan.find(chan_name)->second.push_back(temp);
                                                                sendtoAll(fd, chan[chan_name], ":" + _m_fd2client[fd].getCprefix() + " JOIN :" + chan_name);
                                                                dispMemberName(fd, chan_name);

                                                        }
                                                        else {
                                                                send_err(fd, ERR_BADCHANNELKEY, ":Cannot join channel (+k)\r\n");
                                                        }
                                                }
                                        }
                                        else{
                                                Client *temp = &(_m_fd2client[fd]);
                                                std::vector<Client *> _v_cli_tmp;
                                                _v_cli_tmp.push_back(temp);
                                                chan_oper[chan_name].push_back(temp);
                                                chan[chan_name] = _v_cli_tmp;
                                                if ((msg.getParams().size() == 2) && (*it_pass != "")) { //creation d'un chan a mdp
                                                        chan_flag[chan_name] = "+m";
                                                        chan_pass[chan_name] = msg.getParams().back();
                                                }
                                                sendtoAll(fd, chan[chan_name], ":" +  _m_fd2client[fd].getCprefix() + " JOIN :" + chan_name);
                                                dispMemberName(fd, chan_name);
                                        }
                                }
                                else{
                                        send_err(fd, ERR_NOSUCHCHANNEL, ":No such channel\r\n");
                                }
                        }
                        else{
                                send_err(fd, ERR_NEEDMOREPARAMS, ":Not enough parameters\r\n");
                        }
                        it_chan++;
                        it_pass++;
                }
        }
        else{
                send_err(fd, ERR_NEEDMOREPARAMS, ":Syntax error\r\n");
        }
        return(0);
}

void Server::send_err(int fd, std::string err, std::string msg) {
        send_privmsg(fd, ":" + static_cast<std::string>(SERV_NAME) + " " + err + msg);
}

int Server::do_cmd(Message msg, int fd){
        if (msg.getCmd() == "PASS") {
                passcmd(msg, fd);
        }
        else if (_m_fd2client[fd].getCorr() == true) {
                if (msg.getCmd() == "NICK") {
                        nickcmd(msg, fd);
                }
                else if (msg.getCmd() == "USER") {
                        usercmd(msg, fd);
                }
                else if (_m_fd2client[fd].getReg() == true)
                {
                        if (msg.getCmd() == "JOIN") {
                                joincmd(msg, fd);
                        }
                        else if (msg.getCmd() == "PRIVMSG" || msg.getCmd() == "NOTICE") {
                                privmsg(msg, fd);
                        }
                        else if (msg.getCmd() == "OPER") {
                                opercmd(msg, fd);
                        }
                        else if (msg.getCmd() == "LIST") {
                                listcmd(msg, fd);
                        }
                        else if (msg.getCmd() == "KICK") {
                                kickcmd(msg, fd);
                        }
                        else if (msg.getCmd() == "NAMES") {
                                namecmd(msg, fd);
                        }
                        else if (msg.getCmd() == "PING") {
                                pingcmd(msg, fd);
                        }
                        else if (msg.getCmd() == "QUIT") {
                                quitcmd(fd);
                        }
                        else if (msg.getCmd() == "MODE") {
                                modecmd(msg, fd);
                        }
                        else if (msg.getCmd() == "PART") {
                                partcmd(msg, fd);
                        }
                        if (_m_fd2client[fd].getOp() == true) {
                                if (msg.getCmd() == "KILL")
                                        killcmd(msg, fd);
                                else if (msg.getCmd() == "SQUIT")
                                        squitcmd(msg, fd);
                        }
                }
                else
                        send_err(fd, ERR_UNKNOWNCOMMAND, " :Unknown command\r\n");
        }
        else {
                send_err(fd, ERR_NOTREGISTERED, "* :Connection not registered\r\n");
                //join privmsg(#chan || nick) OPER-> KILL
        }
        return (0);
}

// verif if username already exists
// otherwise send error
//
