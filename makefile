CC = gcc
CFLAGS = -Wall -g

# Target executable names
SERVER = TFTP_Server
CLIENT = TFTP_Client

# Source files
SERVER_SRC = TFTP_Server.c
CLIENT_SRC = TFTP_Client.c
HEADER = udp_file_transfer.h

# Object files
SERVER_OBJ = $(SERVER_SRC:.c=.o)
CLIENT_OBJ = $(CLIENT_SRC:.c=.o)

# Compiler rules
all: $(SERVER) $(CLIENT)

$(SERVER): $(SERVER_OBJ) 
	$(CC) $(CFLAGS) -o $(SERVER) $(SERVER_OBJ)

$(CLIENT): $(CLIENT_OBJ)
	$(CC) $(CFLAGS) -o $(CLIENT) $(CLIENT_OBJ)

# Generic rule to compile .c to .o
%.o: %.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up object files and executables
clean:
	rm -f $(SERVER) $(CLIENT) $(SERVER_OBJ) $(CLIENT_OBJ)

# Rebuild everything from scratch
rebuild: clean all