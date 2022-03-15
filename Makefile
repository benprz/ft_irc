#MAKEFLAGS += --silent

NAME		= ircserv

CC			= c++

CPPFLAGS	= -g3 -std=c++98

SRCS		=	$(wildcard *.cpp)

HEADER		=	$(wildcard *.hpp)

.PHONY		:	all clean fclean re exec

all			:	$(NAME)

$(NAME)		:	$(SRCS) $(HEADER)
				$(CC) $(CPPFLAGS) $(SRCS) -o $(NAME)

clean		:
				rm -rf $(NAME)

fclean		:	clean

re			: 
				$(MAKE) fclean
				$(MAKE) all
