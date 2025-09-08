# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = $(LEX_DIR)/c-tds

# Directorios
LEX_DIR = lexico_sintactico

# Archivos de entrada para Bison y Flex
LEX_SRC = $(LEX_DIR)/lexer.l
YACC_SRC = $(LEX_DIR)/parser.y

# Archivos generados por Bison y Flex
LEX_OUT = $(LEX_DIR)/lex.yy.c
YACC_C = $(LEX_DIR)/parser.tab.c
YACC_H = $(LEX_DIR)/parser.tab.h

# Variable para testeo
TEST ?= tests/test1.txt

# Regla principal
all: $(TARGET)

# Compilacion
$(TARGET): $(YACC_C) $(LEX_OUT)
	$(CC) $(CFLAGS) -o $@ $^ -lfl

$(YACC_C) $(YACC_H): $(YACC_SRC)
	cd $(LEX_DIR) && bison -d parser.y

$(LEX_OUT): $(LEX_SRC) $(YACC_H)
	cd $(LEX_DIR) && flex lexer.l

# Ejecutar el programa
run: $(TARGET)
	@if [ -f "$(TEST)" ]; then \
		echo "▶️ Ejecutando test: $(TEST)"; \
		./$(TARGET) $(TEST); \
	else \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(LEX_OUT) $(YACC_C) $(YACC_H)

.PHONY: all run clean