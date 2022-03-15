#MAKEFLAGS += --silent

NAME		= ircserv

CC			= c++

CPPFLAGS	= -g3 -std=c++98

INC_DIR		= ./

SRCS		=	$(wildcard *.cpp)

HEADER		=	$(wildcard *.hpp)

SRC_DIR		=	./


OBJ_DIR		=	.obj/
OBJ			=	$(SRC:%.cpp=$(OBJ_DIR)%.o)

.PHONY		:	all clean fclean re exec

all			:	$(NAME)

$(NAME)		:	$(SRCS) $(HEADER)
				$(CC) $(CPPFLAGS) $(SRCS) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(addprefix $(INC_DIR),$(INC))
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -I$(INC_DIR) -c $< -o $@

# exec:
#	 ./$(NAME)

clean:
	/bin/rm -rf $(OBJ_DIR)

fclean: clean
	/bin/rm -f $(NAME)

re: 
	$(MAKE) fclean
	$(MAKE) all
