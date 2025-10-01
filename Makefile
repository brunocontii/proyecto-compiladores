# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = c-tds

# Directorios
LEX_DIR = lexico_sintactico
TREE_DIR = arbol-sintactico
SIMBOLOS_DIR = tabla-simbolos
SEMANTICO_DIR = analisis-semantico
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
SEMANTICO_SRC = $(SEMANTICO_DIR)/semantico.c
UTILS_SRC = $(UTILS_DIR)/manejo_errores.c

# Main
MAIN_SRC = main.c

# Variable para testeo
TEST ?= tests/tests-semantico/test1.ctds

# Regla principal
all: $(TARGET)

# Compilacion
$(TARGET): $(MAIN_SRC) $(YACC_C) $(LEX_OUT) $(TREE_SRC) $(SIMBOLOS_SRC) $(SEMANTICO_SRC) $(UTILS_SRC)
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

# Ejecutar todos los tests (sintácticos y semánticos)
test-all: $(TARGET)
	@echo "=== EJECUTANDO TODOS LOS TESTS ==="
	@passed=0; failed=0; total=0; \
	for test in tests/tests-sintactico/*.ctds tests/tests-semantico/*.ctds; do \
		if [ -f "$$test" ]; then \
			total=$$((total + 1)); \
			basename_test=$$(basename "$$test"); \
			dirname_test=$$(basename $$(dirname "$$test")); \
			./$(TARGET) "$$test" > /tmp/test_output.txt 2>&1; \
			exit_code=$$?; \
			if [ $$exit_code -eq 0 ] && grep -q "EXITOSA" /tmp/test_output.txt; then \
				printf "\033[32m📝 Test %2d: [%-15s] %-20s ✅ PASÓ\033[0m\n" "$$total" "$$dirname_test" "$$basename_test"; \
				passed=$$((passed + 1)); \
			else \
				printf "\033[31m📝 Test %2d: [%-15s] %-20s ❌ FALLÓ\033[0m\n" "$$total" "$$dirname_test" "$$basename_test"; \
				failed=$$((failed + 1)); \
			fi; \
		fi; \
	done; \
	echo ""; \
	echo "Total tests ejecutados: $$total"; \
	echo "\033[32m✅ Pasaron: $$passed\033[0m"; \
	echo "\033[31m❌ Fallaron: $$failed\033[0m"; \
	if [ $$failed -eq 0 ]; then \
		echo "\033[32m🎉 TODOS LOS TESTS PASARON\033[0m"; \
	fi

# Ejecutar solo tests sintácticos
test-sintactico: $(TARGET)
	@echo "=== EJECUTANDO TESTS SINTÁCTICOS ==="
	@passed=0; failed=0; total=0; \
	for test in tests/tests-sintactico/*.ctds; do \
		if [ -f "$$test" ]; then \
			total=$$((total + 1)); \
			basename_test=$$(basename "$$test"); \
			./$(TARGET) -target parse "$$test" > /tmp/test_output.txt 2>&1; \
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
	echo "=== RESUMEN TESTS SINTÁCTICOS ==="; \
	echo "Total tests ejecutados: $$total"; \
	echo "\033[32m✅ Pasaron: $$passed\033[0m"; \
	echo "\033[31m❌ Fallaron: $$failed\033[0m"

# Ejecutar solo tests semánticos
test-semantico: $(TARGET)
	@echo "=== EJECUTANDO TESTS SEMÁNTICOS ==="
	@passed=0; failed=0; total=0; \
	for test in tests/tests-semantico/*.ctds; do \
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
	echo "=== RESUMEN TESTS SEMÁNTICOS ==="; \
	echo "Total tests ejecutados: $$total"; \
	echo "\033[32m✅ Pasaron: $$passed\033[0m"; \
	echo "\033[31m❌ Fallaron: $$failed\033[0m"

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(LEX_OUT) $(YACC_C) $(YACC_H) *.dot *.png

.PHONY: all run clean test-all test-sintactico test-semantico