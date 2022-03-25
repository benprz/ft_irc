NAME =	irc

SRCSPATH =	src/
#DEBUGPATH =	debug/
INCPATH =	inc/

#---------	INCLUDES --
INCLUDES =				$(INCPATH)ClientsMonitoringList.hpp \
						$(INCPATH)Server.hpp

#--------	SRCS --
SRCS = 		$(SRCSPATH)main.cpp \
					$(SRCSPATH)ClientsMonitoringList.cpp \
					$(SRCSPATH)Server.cpp \

#--------	COMP --

CC =		clang++

CFLAGSPROD	= -Wall -Wextra -Werror -std=c++98
CFLAGS		= -Wall -Wextra -Werror
CFLAGSPADD	= -Wpadded
CFLAGSSAN	=
ACTIVES_FLAGS = $(CFLAGSSAN) $(CFLAGSPROD)

OBJS = ${SRCS:.cpp=.o}

$(NAME):	$(OBJS) $(INCLUDES)
					$(CC) $(CFLAGS) $(CFLAGSSAN) $(OBJS) -o $(NAME)

$(OBJS):	$(INCLUDES)

all:		$(NAME)

launch:		$(NAME)
					./$(NAME)

clean:
			${RM} $(OBJS)

fclean:		clean
			${RM} $(NAME).a $(NAME)

re:			fclean all

.PHONY: clear
