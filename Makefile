#MAKEFLAGS += --silent

NAME = ircserv
CC = clang++
CPPFLAGS = -g3 -std=c++98
INC_DIR = inc/
INC =	Server.hpp \
		NumericReplies.hpp \
		ChannelsList.hpp

SRC_DIR = src/
SRC =	main.cpp \
		Server.cpp \
		Commands.cpp \
		ChannelsList.cpp

OBJ_DIR = .obj/
OBJ = $(SRC:%.cpp=$(OBJ_DIR)%.o)

.PHONY : all clean fclean re exec

all: $(NAME) exec

$(NAME): $(OBJ)
	$(CC) $(CFLAGS) -I$(INC_DIR) $(OBJ) -o $(NAME)

$(OBJ_DIR)%.o: $(SRC_DIR)%.cpp $(addprefix $(INC_DIR),$(INC))
	mkdir -p $(@D)
	$(CC) $(CPPFLAGS) -I$(INC_DIR) -c $< -o $@

exec:
	./$(NAME) 2000 pw

clean:
	/bin/rm -rf $(OBJ_DIR)

fclean: clean
	/bin/rm -f $(NAME)

re: 
	$(MAKE) fclean
	$(MAKE) all
