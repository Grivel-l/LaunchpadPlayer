NAME = pad

SRCS_PATH = ./srcs/
OBJS_PATH = ./objs/
INCS_PATH = ./includes/

SRCS = main.c
OBJS = $(addprefix $(OBJS_PATH)/, $(SRCS:.c=.o))

CC = gcc
FLAGS = -Wall -Wextra -Werror -lpthread -lusb-1.0 -lSDL2 -lSDL_mixer

.PHONY: all clean fclean re

all: $(NAME)

$(NAME): $(OBJS_PATH) $(OBJS)
	$(CC) $(FLAGS) $(OBJS) -o $(NAME) -I $(INCS_PATH)

$(OBJS_PATH):
	mkdir -p $(OBJS_PATH)

$(OBJS_PATH)/%.o: $(SRCS_PATH)/%.c
	$(CC) $(FLAGS) -c $< -o $@ -I $(INCS_PATH)

clean:
	rm -rf $(OBJS_PATH)

fclean: clean
	rm -f $(NAME)

re: fclean all

