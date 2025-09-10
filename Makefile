# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = $(LEX_DIR)/c-tds

# Directorios
LEX_DIR = lexico_sintactico
TREE_DIR = arbol-sintactico

# Archivos de entrada para Bison y Flex
LEX_SRC = $(LEX_DIR)/lexer.l
YACC_SRC = $(LEX_DIR)/parser.y

# Archivos generados por Bison y Flex
LEX_OUT = $(LEX_DIR)/lex.yy.c
YACC_C = $(LEX_DIR)/parser.tab.c
YACC_H = $(LEX_DIR)/parser.tab.h

# Archivos adicionales
TREE_SRC = $(TREE_DIR)/arbol.c  $(TREE_DIR)/image_ast.c

# Variable para testeo
TEST ?= tests/test1.ctds

# Regla principal
all: $(TARGET)

# Compilacion
$(TARGET): $(YACC_C) $(LEX_OUT) $(TREE_SRC)
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
	rm -f $(TARGET) $(LEX_OUT) $(YACC_C) $(YACC_H) ctds_arbol.dot ctds_arbol.png

.PHONY: all run clean