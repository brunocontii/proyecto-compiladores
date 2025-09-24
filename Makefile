# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = c-tds

# Directorios
LEX_DIR = lexico_sintactico
TREE_DIR = arbol-sintactico
SIMBOLOS_DIR = tabla-simbolos
UTILS_DIR = utils

# Archivos de entrada para Bison y Flex
LEX_SRC = $(LEX_DIR)/lexer.l
YACC_SRC = $(LEX_DIR)/parser.y

# Archivos generados por Bison y Flex
LEX_OUT = $(LEX_DIR)/lex.yy.c
YACC_C = $(LEX_DIR)/parser.tab.c
YACC_H = $(LEX_DIR)/parser.tab.h

# Archivos adicionales
TREE_SRC = $(TREE_DIR)/arbol.c  $(TREE_DIR)/image_ast.c
SIMBOLOS_SRC = $(SIMBOLOS_DIR)/tabla_simbolos.c
UTILS_SRC = $(UTILS_DIR)/manejo_errores.c

# Main
MAIN_SRC = main.c

# Variable para testeo
TEST ?= tests/test1.ctds

# Regla principal
all: $(TARGET)

# Compilacion
$(TARGET): $(MAIN_SRC) $(YACC_C) $(LEX_OUT) $(TREE_SRC) $(SIMBOLOS_SRC) $(UTILS_SRC)
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

# Ejecutar todos los tests
test-all: $(TARGET)
	@echo "=== EJECUTANDO TODOS LOS TESTS ==="
	@passed=0; failed=0; total=0; \
	for test in tests/*.ctds; do \
		if [ -f "$$test" ]; then \
			total=$$((total + 1)); \
			basename_test=$$(basename "$$test"); \
			./$(TARGET) "$$test" > /tmp/test_output.txt 2>&1; \
			exit_code=$$?; \
			if [ $$exit_code -eq 0 ] && grep -q "EXITOSA" /tmp/test_output.txt; then \
				printf "\033[32m📝 Test %2d: %-20s ✅ PASÓ\033[0m\n" "$$total" "$$basename_test"; \
				passed=$$((passed + 1)); \
			else \
				printf "\033[31m📝 Test %2d: %-20s ❌ FALLÓ\033[0m\n" "$$total" "$$basename_test"; \
				failed=$$((failed + 1)); \
			fi; \
		fi; \
	done; \
	echo ""; \
	echo "=== RESUMEN FINAL ==="; \
	echo "Total tests ejecutados: $$total"; \
	echo "\033[32m✅ Pasaron: $$passed\033[0m"; \
	echo "\033[31m❌ Fallaron: $$failed\033[0m"; \
	if [ $$failed -eq 0 ]; then \
		echo "\033[32m🎉 TODOS LOS TESTS PASARON\033[0m"; \
	else \
		echo "\033[33m⚠️  $$failed test(s) fallaron\033[0m"; \
	fi

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(LEX_OUT) $(YACC_C) $(YACC_H) *.dot *.png

.PHONY: all run clean test-all