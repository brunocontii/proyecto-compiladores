# Compilador y flags
CC = gcc
CFLAGS = -Wall -Wextra -g
TARGET = c-tds

# Directorios
LEX_DIR = lexico_sintactico
TREE_DIR = arbol-sintactico
SIMBOLOS_DIR = tabla-simbolos
SEMANTICO_DIR = analisis-semantico
CI_DIR = codigo-intermedio
ASSEMBLER_DIR = assembler
OPT_DIR = optimizaciones

# Archivos de entrada para Bison y Flex
LEX_SRC = $(LEX_DIR)/lexer.l
YACC_SRC = $(LEX_DIR)/parser.y

# Archivos generados por Bison y Flex
LEX_OUT = $(LEX_DIR)/lex.yy.c
YACC_C = $(LEX_DIR)/parser.tab.c
YACC_H = $(LEX_DIR)/parser.tab.h

# Archivos adicionales
TREE_SRC = 	$(TREE_DIR)/arbol.c \
			$(TREE_DIR)/image_ast.c

SIMBOLOS_SRC = $(SIMBOLOS_DIR)/tabla_simbolos.c

SEMANTICO_SRC = $(SEMANTICO_DIR)/semantico.c \
				$(SEMANTICO_DIR)/manejo_errores.c

CI_SRC = 	$(CI_DIR)/generador.c \
			$(CI_DIR)/codigo3dir.c \
			$(CI_DIR)/auxiliares.c \
			$(CI_DIR)/parametros.c

ASSEMBLER_SRC = $(ASSEMBLER_DIR)/assembler.c \
				$(ASSEMBLER_DIR)/metodos.c \
				$(ASSEMBLER_DIR)/parametros.c \
				$(ASSEMBLER_DIR)/secciones.c \
				$(ASSEMBLER_DIR)/variables.c \
				$(ASSEMBLER_DIR)/instrucciones.c

OPTIMIZACIONES_SRC = $(OPT_DIR)/optimizaciones.c \
					 $(OPT_DIR)/plegado_constantes.c \
					 $(OPT_DIR)/operaciones.c \
					 $(OPT_DIR)/codigo_muerto.c \
					 $(OPT_DIR)/codigo_muerto_var.c \
					 $(OPT_DIR)/codigo_muerto_bloque.c

RUNTIME_DIR = runtime
RUNTIME_SRC = $(RUNTIME_DIR)/func-extern.c

# Main
MAIN_SRC = main.c

# Variables configurables para run
TEST ?= tests/tests-semantico/test01.ctds
OPT ?=
TEST_OPT ?=
ALL_OPTS = prop-constantes operaciones cod-inalcanzable var-muertas cod-bloque

# Construir flags de optimización (soporta múltiples, separadas por comas)
OPT_FLAGS =
ifneq ($(OPT),)
	# Convertir comas en espacios y crear -opt para cada una
	COMMA := ,
	EMPTY :=
	SPACE := $(EMPTY) $(EMPTY)
	OPT_LIST := $(subst $(COMMA),$(SPACE),$(OPT))
	OPT_FLAGS := $(foreach opt,$(OPT_LIST),-opt $(opt))
endif

# Regla principal
all: $(TARGET)

# Compilacion
$(TARGET): $(MAIN_SRC) $(YACC_C) $(LEX_OUT) $(TREE_SRC) $(SIMBOLOS_SRC) $(SEMANTICO_SRC) $(CI_SRC) $(ASSEMBLER_SRC) $(OPTIMIZACIONES_SRC)
	$(CC) $(CFLAGS) -o $@ $^ -lfl

$(YACC_C) $(YACC_H): $(YACC_SRC)
	cd $(LEX_DIR) && bison -d parser.y

$(LEX_OUT): $(LEX_SRC) $(YACC_H)
	cd $(LEX_DIR) && flex lexer.l

# Target run genérico
run: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Compilando: $(TEST)"
	@if [ -n "$(OPT)" ]; then \
		echo "🔧 Optimizaciones: $(OPT)"; \
	fi
	./$(TARGET) -target assembly $(OPT_FLAGS) $(TEST)
	@if [ -f assembler.s ]; then \
		echo "🔧 Generando ejecutable..."; \
		gcc -g assembler.s $(RUNTIME_SRC) -o prog; \
		echo "🚀 Ejecutando programa:"; \
		./prog; \
	fi

# Shortcuts para cada etapa (SIN optimización por defecto)
run-lex: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Análisis Léxico: $(TEST)"
	./$(TARGET) -target lex $(TEST)

run-parse: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Análisis Sintáctico: $(TEST)"
	./$(TARGET) -target parse $(TEST)

run-sem: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Análisis Semántico: $(TEST)"
	@if [ -n "$(OPT)" ]; then \
		echo "🔧 Optimizaciones: $(OPT)"; \
	fi
	./$(TARGET) -target sem $(OPT_FLAGS) $(TEST)

run-ci: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Código Intermedio: $(TEST)"
	@if [ -n "$(OPT)" ]; then \
		echo "🔧 Optimizaciones: $(OPT)"; \
	fi
	./$(TARGET) -target codinter $(OPT_FLAGS) $(TEST)

run-asm: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Assembler: $(TEST)"
	@if [ -n "$(OPT)" ]; then \
		echo "🔧 Optimizaciones: $(OPT)"; \
	fi
	./$(TARGET) -target assembly $(OPT_FLAGS) $(TEST)
	@if [ -f assembler.s ]; then \
		echo "🔧 Generando ejecutable..."; \
		gcc -g assembler.s $(RUNTIME_SRC) -o prog; \
		echo "🚀 Ejecutando programa:"; \
		./prog; \
	fi

# Ejecutar con todas las optimizaciones
run-all-opt: $(TARGET)
	@if [ ! -f "$(TEST)" ]; then \
		echo "❌ ERROR: El archivo $(TEST) no existe"; \
		exit 1; \
	fi
	@echo "▶️  Assembler con todas las optimizaciones: $(TEST)"
	@echo "🔧 Optimizaciones: $(ALL_OPTS)"
	./$(TARGET) -target assembly -opt prop-constantes -opt operaciones -opt cod-inalcanzable -opt var-muertas -opt cod-bloque $(TEST)
	@if [ -f assembler.s ]; then \
		echo "🔧 Generando ejecutable..."; \
		gcc -g assembler.s $(RUNTIME_SRC) -o prog; \
		echo "🚀 Ejecutando programa:"; \
		./prog; \
	fi

# Tests
test-assembler: $(TARGET)
	@echo "=== EJECUTANDO TESTS ASSEMBLER ==="
	@passed=0; failed=0; total=0; \
	for test in tests/tests-assembler/*.ctds; do \
		if [ -f "$$test" ]; then \
			total=$$((total + 1)); \
			basename_test=$$(basename "$$test"); \
			\
			./$(TARGET) -target assembly "$$test" > /tmp/compiler_output.txt 2>&1; \
			compile_exit=$$?; \
			\
			if [ $$compile_exit -ne 0 ] || [ ! -f assembler.s ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR DE COMPILACIÓN\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Error del compilador:"; \
				cat /tmp/compiler_output.txt | sed 's/^/      /'; \
				failed=$$((failed + 1)); \
				continue; \
			fi; \
			\
			gcc -no-pie assembler.s $(RUNTIME_SRC) -o prog > /tmp/gcc_output.txt 2>&1; \
			gcc_exit=$$?; \
			\
			if [ $$gcc_exit -ne 0 ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR EN GCC\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Error de ensamblado:"; \
				cat /tmp/gcc_output.txt | sed 's/^/      /'; \
				failed=$$((failed + 1)); \
				rm -f assembler.s; \
				continue; \
			fi; \
			\
			output=$$(./prog 2>&1); \
			run_exit=$$?; \
			\
			if [ $$run_exit -ne 0 ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR EN EJECUCIÓN\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Código de salida: $$run_exit"; \
				echo "   └─ Salida: $$output"; \
				failed=$$((failed + 1)); \
			elif echo "$$output" | grep -q "^true$$"; then \
				printf "\033[32m📋 Test %2d: %-35s ✅ PASÓ\033[0m\n" "$$total" "$$basename_test"; \
				passed=$$((passed + 1)); \
			elif echo "$$output" | grep -q "^false$$"; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ FALLÓ (test retornó false)\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ El programa indicó que el test falló"; \
				failed=$$((failed + 1)); \
			else \
				printf "\033[33m📋 Test %2d: %-35s ⚠️  SALIDA INESPERADA\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Esperado: true"; \
				echo "   └─ Obtenido: $$output"; \
				failed=$$((failed + 1)); \
			fi; \
			\
			rm -f assembler.s prog; \
		fi; \
	done; \
	echo ""; \
	echo "RESUMEN DE TESTS"; \
	printf " Total de tests:      %3d \n" "$$total"; \
	printf " \033[32m✅ Tests pasados:    %3d\033[0m \n" "$$passed"; \
	printf " \033[31m❌ Tests fallados:   %3d\033[0m \n" "$$failed"; \
	echo ""; \
	if [ $$failed -gt 0 ]; then \
		exit 1; \
	fi


# Tests con optimizaciones específicas
test-assembler-opt: $(TARGET)
	@if [ -z "$(TEST_OPT)" ]; then \
		echo "❌ ERROR: Debes especificar TEST_OPT=<optimización>"; \
		echo "Opciones:"; \
		echo "  TEST_OPT=prop-constantes"; \
		echo "  TEST_OPT=operaciones"; \
		echo "  TEST_OPT=cod-inalcanzable"; \
		echo "  TEST_OPT=var-muertas"; \
		echo "  TEST_OPT=all  (todas las optimizaciones)"; \
		exit 1; \
	fi
	@if [ "$(TEST_OPT)" = "all" ]; then \
		echo "=== TESTS ASSEMBLER CON TODAS LAS OPTIMIZACIONES ==="; \
		OPT_FLAGS="-opt prop-constantes -opt operaciones -opt cod-inalcanzable -opt var-muertas"; \
	else \
		echo "=== TESTS ASSEMBLER CON OPTIMIZACIÓN: $(TEST_OPT) ==="; \
		OPT_FLAGS="-opt $(TEST_OPT)"; \
	fi; \
	passed=0; failed=0; total=0; \
	for test in tests/tests-assembler/*.ctds; do \
		if [ -f "$$test" ]; then \
			total=$$((total + 1)); \
			basename_test=$$(basename "$$test"); \
			\
			./$(TARGET) -target assembly $$OPT_FLAGS "$$test" > /tmp/compiler_output.txt 2>&1; \
			compile_exit=$$?; \
			\
			if [ $$compile_exit -ne 0 ] || [ ! -f assembler.s ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR DE COMPILACIÓN\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Error del compilador:"; \
				cat /tmp/compiler_output.txt | sed 's/^/      /'; \
				failed=$$((failed + 1)); \
				continue; \
			fi; \
			\
			gcc -no-pie assembler.s $(RUNTIME_SRC) -o prog > /tmp/gcc_output.txt 2>&1; \
			gcc_exit=$$?; \
			\
			if [ $$gcc_exit -ne 0 ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR EN GCC\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Error de ensamblado:"; \
				cat /tmp/gcc_output.txt | sed 's/^/      /'; \
				failed=$$((failed + 1)); \
				rm -f assembler.s; \
				continue; \
			fi; \
			\
			output=$$(./prog 2>&1); \
			run_exit=$$?; \
			\
			if [ $$run_exit -ne 0 ]; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ ERROR EN EJECUCIÓN\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Código de salida: $$run_exit"; \
				echo "   └─ Salida: $$output"; \
				failed=$$((failed + 1)); \
			elif echo "$$output" | grep -q "^1$$"; then \
				printf "\033[32m📋 Test %2d: %-35s ✅ PASÓ\033[0m\n" "$$total" "$$basename_test"; \
				passed=$$((passed + 1)); \
			elif echo "$$output" | grep -q "^0$$"; then \
				printf "\033[31m📋 Test %2d: %-35s ❌ FALLÓ (test retornó false)\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ El programa indicó que el test falló"; \
				failed=$$((failed + 1)); \
			else \
				printf "\033[33m📋 Test %2d: %-35s ⚠️  SALIDA INESPERADA\033[0m\n" "$$total" "$$basename_test"; \
				echo "   └─ Esperado: 1 (true)"; \
				echo "   └─ Obtenido: $$output"; \
				failed=$$((failed + 1)); \
			fi; \
			\
			rm -f assembler.s prog; \
		fi; \
	done; \
	echo ""; \
	echo "RESUMEN DE TESTS"; \
	printf " Total de tests:      %3d \n" "$$total"; \
	printf " \033[32m✅ Tests pasados:    %3d\033[0m \n" "$$passed"; \
	printf " \033[31m❌ Tests fallados:   %3d\033[0m \n" "$$failed"; \
	echo ""; \
	if [ $$failed -gt 0 ]; then \
		exit 1; \
	fi

# Comparación: ejecutar tests sin y con optimizaciones
test-optimizacion-compare: $(TARGET)
	@echo "=== COMPARACIÓN DE OPTIMIZACIONES ==="
	@echo "📊 Contando líneas de código 3D (sin optimizar vs optimizado)"
	@echo ""
	@if [ ! -d "tests/tests-optimizacion" ]; then \
		echo "❌ ERROR: No existe la carpeta tests/tests-optimizacion"; \
		exit 1; \
	fi
	@total_tests=0; \
	total_sin_opt=0; \
	total_con_opt=0; \
	printf "%-30s %15s %15s %15s\n" "TEST" "SIN OPT" "CON OPT" "REDUCCIÓN"; \
	echo "--------------------------------------------------------------------------------"; \
	for test in tests/tests-optimizacion/*.ctds; do \
		if [ -f "$$test" ]; then \
			total_tests=$$((total_tests + 1)); \
			basename_test=$$(basename "$$test"); \
			\
			./$(TARGET) -target codinter "$$test" > /tmp/sin_opt.txt 2>&1; \
			if [ $$? -ne 0 ]; then \
				printf "%-30s %15s %15s %15s\n" "$$basename_test" "ERROR" "-" "-"; \
				continue; \
			fi; \
			\
			lineas_sin_opt=$$(grep -c "^[0-9]" /tmp/sin_opt.txt 2>/dev/null || echo "0"); \
			\
			./$(TARGET) -target codinter -opt prop-constantes -opt operaciones -opt cod-inalcanzable -opt var-muertas "$$test" > /tmp/con_opt.txt 2>&1; \
			if [ $$? -ne 0 ]; then \
				printf "%-30s %15s %15s %15s\n" "$$basename_test" "$$lineas_sin_opt" "ERROR" "-"; \
				continue; \
			fi; \
			\
			lineas_con_opt=$$(grep -c "^[0-9]" /tmp/con_opt.txt 2>/dev/null || echo "0"); \
			\
			if [ $$lineas_sin_opt -gt 0 ]; then \
				reduccion=$$((lineas_sin_opt - lineas_con_opt)); \
				porcentaje=$$(awk "BEGIN {printf \"%.1f\", ($$reduccion / $$lineas_sin_opt) * 100}"); \
				if [ $$reduccion -gt 0 ]; then \
					printf "%-30s %15s %15s \033[32m%10s (-%.0f%%)\033[0m\n" "$$basename_test" "$$lineas_sin_opt líneas" "$$lineas_con_opt líneas" "$$reduccion líneas" "$$porcentaje"; \
				elif [ $$reduccion -lt 0 ]; then \
					printf "%-30s %15s %15s \033[31m%10s (+%.0f%%)\033[0m\n" "$$basename_test" "$$lineas_sin_opt líneas" "$$lineas_con_opt líneas" "$$reduccion líneas" "$$porcentaje"; \
				else \
					printf "%-30s %15s %15s %15s\n" "$$basename_test" "$$lineas_sin_opt líneas" "$$lineas_con_opt líneas" "Sin cambios"; \
				fi; \
			else \
				printf "%-30s %15s %15s %15s\n" "$$basename_test" "0 líneas" "0 líneas" "Sin cambios"; \
			fi; \
			\
			total_sin_opt=$$((total_sin_opt + lineas_sin_opt)); \
			total_con_opt=$$((total_con_opt + lineas_con_opt)); \
		fi; \
	done; \
	echo "--------------------------------------------------------------------------------"; \
	reduccion_total=$$((total_sin_opt - total_con_opt)); \
	if [ $$total_sin_opt -gt 0 ]; then \
		porcentaje_total=$$(awk "BEGIN {printf \"%.1f\", ($$reduccion_total / $$total_sin_opt) * 100}"); \
		printf "%-30s %15s %15s \033[1;32m%10s (-%.1f%%)\033[0m\n" "TOTAL ($$total_tests tests)" "$$total_sin_opt líneas" "$$total_con_opt líneas" "$$reduccion_total líneas" "$$porcentaje_total"; \
	else \
		printf "%-30s %15s %15s %15s\n" "TOTAL ($$total_tests tests)" "0 líneas" "0 líneas" "Sin cambios"; \
	fi; \
	echo ""; \
	rm -f /tmp/sin_opt.txt /tmp/con_opt.txt

# Test detallado: muestra cada optimización por separado
test-optimizacion-detalle: $(TARGET)
	@echo "=== ANÁLISIS DETALLADO DE OPTIMIZACIONES ==="
	@echo ""
	@if [ ! -d "tests/tests-optimizacion" ]; then \
		echo "❌ ERROR: No existe la carpeta tests/tests-optimizacion"; \
		exit 1; \
	fi
	@for test in tests/tests-optimizacion/*.ctds; do \
		if [ -f "$$test" ]; then \
			basename_test=$$(basename "$$test"); \
			echo "📄 Test: $$basename_test"; \
			echo "----------------------------------------"; \
			\
			./$(TARGET) -target codinter "$$test" > /tmp/sin_opt.txt 2>&1; \
			lineas_sin_opt=$$(grep -c "^[0-9]" /tmp/sin_opt.txt 2>/dev/null || echo "0"); \
			printf "  Sin optimizar:              %3d líneas\n" "$$lineas_sin_opt"; \
			\
			./$(TARGET) -target codinter -opt prop-constantes "$$test" > /tmp/opt1.txt 2>&1; \
			lineas_opt1=$$(grep -c "^[0-9]" /tmp/opt1.txt 2>/dev/null || echo "0"); \
			diff1=$$((lineas_sin_opt - lineas_opt1)); \
			printf "  + prop-constantes:          %3d líneas (%+d)\n" "$$lineas_opt1" "$$diff1"; \
			\
			./$(TARGET) -target codinter -opt prop-constantes -opt operaciones "$$test" > /tmp/opt2.txt 2>&1; \
			lineas_opt2=$$(grep -c "^[0-9]" /tmp/opt2.txt 2>/dev/null || echo "0"); \
			diff2=$$((lineas_opt1 - lineas_opt2)); \
			printf "  + operaciones:              %3d líneas (%+d)\n" "$$lineas_opt2" "$$diff2"; \
			\
			./$(TARGET) -target codinter -opt prop-constantes -opt operaciones -opt cod-inalcanzable "$$test" > /tmp/opt3.txt 2>&1; \
			lineas_opt3=$$(grep -c "^[0-9]" /tmp/opt3.txt 2>/dev/null || echo "0"); \
			diff3=$$((lineas_opt2 - lineas_opt3)); \
			printf "  + cod-inalcanzable:         %3d líneas (%+d)\n" "$$lineas_opt3" "$$diff3"; \
			\
			./$(TARGET) -target codinter -opt prop-constantes -opt operaciones -opt cod-inalcanzable -opt var-muertas "$$test" > /tmp/opt4.txt 2>&1; \
			lineas_opt4=$$(grep -c "^[0-9]" /tmp/opt4.txt 2>/dev/null || echo "0"); \
			diff4=$$((lineas_opt3 - lineas_opt4)); \
			printf "  + var-muertas:              %3d líneas (%+d)\n" "$$lineas_opt4" "$$diff4"; \
			\
			total_reduccion=$$((lineas_sin_opt - lineas_opt4)); \
			if [ $$lineas_sin_opt -gt 0 ]; then \
				porcentaje=$$(awk "BEGIN {printf \"%.1f\", ($$total_reduccion / $$lineas_sin_opt) * 100}"); \
				printf "  \033[1;32mReducción total:            %3d líneas (%.1f%%)\033[0m\n" "$$total_reduccion" "$$porcentaje"; \
			fi; \
			echo ""; \
		fi; \
	done; \
	rm -f /tmp/sin_opt.txt /tmp/opt1.txt /tmp/opt2.txt /tmp/opt3.txt /tmp/opt4.txt


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

# Ayuda
help:
	@echo "Comandos disponibles del Makefile:"
	@echo ""
	@echo ">> Compilación:"
	@echo "  make                    - Compilar el compilador"
	@echo "  make clean              - Limpiar archivos generados"
	@echo ""
	@echo ">> Ejecución por etapa (sin optimización por defecto):"
	@echo "  make run-lex   TEST=<archivo>           - Análisis léxico"
	@echo "  make run-parse TEST=<archivo>           - Análisis sintáctico"
	@echo "  make run-sem   TEST=<archivo> [OPT=<opt>] - Análisis semántico"
	@echo "  make run-ci    TEST=<archivo> [OPT=<opt>] - Código intermedio"
	@echo "  make run-asm   TEST=<archivo> [OPT=<opt>] - Assembler + ejecutar"
	@echo "  make run       TEST=<archivo> [OPT=<opt>] - Igual que run-asm"
	@echo ""
	@echo ">> Tests (sin optimizaciones):"
	@echo "  make test-all           - Ejecutar todos los tests"
	@echo "  make test-sintactico    - Tests sintácticos"
	@echo "  make test-semantico     - Tests semánticos"
	@echo "  make test-assembler     - Tests de assembler"
	@echo ""
	@echo ">> Tests (con optimizaciones):"
	@echo "  make test-assembler-opt TEST_OPT=<opt>  - Tests con optimización específica"
	@echo "  make test-assembler-opt TEST_OPT=all    - Tests con TODAS las optimizaciones"
	@echo "  make test-optimizacion-compare   		 - Comparar líneas de código 3D (sin vs con opt)"
	@echo "  make test-optimizacion-detalle   		 - Análisis detallado por cada optimización"
	@echo ""
	@echo ">> Optimizaciones disponibles:"
	@echo "  OPT=prop-constantes     - Propagación de constantes"
	@echo "  OPT=var-muertas         - Eliminación de variables no usadas"
	@echo "  OPT=cod-inalcanzable    - Eliminación de código inalcanzable"
	@echo "  OPT=var-muertas         - Eliminación de variables no usadas"
	@echo "  OPT=operaciones         - Simplificación de operaciones"
	@echo "  OPT=all                 - Todas las optimizaciones"
	@echo ""
	@echo ">> Ejemplos:"
	@echo "  make run-asm TEST=tests/tests-assembler/test01asm.ctds"
	@echo "  make run-asm TEST=tests/tests-assembler/test01asm.ctds OPT=prop-constantes"
	@echo "  make run-asm TEST=tests/test.ctds OPT=var-muertas"
	@echo "  make test-assembler-opt TEST_OPT=all"
	@echo "  make test-optimizacion-compare"
	@echo "  make test-optimizacion-detalle"
	@echo "  make run-ci TEST=tests/tests-assembler/test02asm.ctds OPT=prop-constantes,var-muertas"
	@echo "  make run-sem TEST=tests/tests-semantico/test03.ctds"

# Limpiar archivos generados
clean:
	rm -f $(TARGET) $(LEX_OUT) $(YACC_C) $(YACC_H) *.dot *.png *.txt *.s prog assembler.s

.PHONY: all run run-lex run-parse run-sem run-ci run-asm run-all-opt \
		clean test-all test-sintactico test-semantico test-assembler \
		test-assembler-opt test-optimizacion-compare test-optimizacion-detalle help