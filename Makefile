#MAKEFLAGS += --silent

NAME		= ircserv

CC			= c++

CPPFLAGS	= -g3 -std=c++98

SRCS		=	$(wildcard *.cpp)

HEADER		=	$(wildcard *.hpp)

.PHONY		:	all clean fclean re exec

all			:	$(NAME) exec

$(NAME)		:	$(SRCS) $(HEADER)
				$(CC) $(CPPFLAGS) $(SRCS) -o $(NAME)

exec		:
				./$(NAME) 16385 pw123

clean		:
				rm -rf $(NAME)

fclean		:	clean

re			: 
				$(MAKE) fclean
				$(MAKE) all
