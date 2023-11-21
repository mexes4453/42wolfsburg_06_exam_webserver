TARGET = server
SRCS = miniserv.c 
#SRCS = main_example.c 
CC = gcc
CFLAGS = -Werror -Wall -Wextra
INCLUDES = -I ./




all: $(TARGET)
$(TARGET) : $(SRCS)
	@echo "Compiling program..."
	$(CC) $^ $(CFLAGS) $(INCLUDES) -o $@

clean:
	rm -f $(TARGET)

fclean:
	rm -f *.o

re: clean all

PHONY. : re all clean fclean