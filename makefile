.PHONY: run
CC         := gcc
DEPENDS    := sqlite3
SRC        := src
INCLUDE    := include
OBJ        := obj
BIN        := bin
CFLAGS     := -g -I$(INCLUDE) -l $(DEPENDS) -D LOG_ENABLED
LDFLAGS    := -l sqlite3
NAME       := main
SOURCES    := $(wildcard $(SRC)/*.c)
OBJECTS    := $(patsubst $(SRC)/%.c, $(OBJ)/%.o, $(SOURCES))

run: $(BIN)/$(NAME)
	@$(BIN)/$(NAME)

$(BIN)/$(NAME): $(OBJECTS)
	$(CC) $(LDFLAGS) $^ -o $(BIN)/$(NAME)

$(OBJ)/%.o: $(SRC)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	@echo "[CLEAN] Cleaning obj and bin..."
	@rm -f $(OBJ)/*
	@rm -f $(BIN)/$(NAME)

show:
	@echo $(CC)
	@echo $(DEPENDS)
	@echo $(SRC)
	@echo $(INCLUDE)
	@echo $(OBJ)
	@echo $(BIN)
	@echo $(CFLAGS)
	@echo $(NAME)
	@echo $(SOURCES)
	@echo $(OBJECTS)