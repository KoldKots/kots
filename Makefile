CXX        = gcc
CXXFLAGS   = -Wall -Wextra -fPIC -w
INCLFLAGS  = -I$(MYSQL_DIR)/include
LDFLAGS    = -shared -L$(MYSQL_DIR)/lib -lpthread -lmysqlclient -lm

# Directories
GAME_DIR   = game
MYSQL_DIR  = /opt/mysql

# Output file
OUTPUT     = gamex64.so

# Source files
SRCS = $(wildcard $(GAME_DIR)/*.cpp) $(wildcard $(GAME_DIR)/*.c)

# Targets
.PHONY: all clean

all: $(SRCS)
	$(CXX) $(CXXFLAGS) $(INCLFLAGS) -o $(OUTPUT) $(SRCS) $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f $(OUTPUT)
	rm -f $(GAME_DIR)/gamex64.so
