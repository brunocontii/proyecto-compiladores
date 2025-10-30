# Proyecto Compiladores - TDS25 🔍
## Codigo Assembly

Esta rama contiene la cuarta entrega del proyecto, implementando código assembly

---

## 👥 Equipo de Desarrollo
- Conti, Bruno  
- Gonzalez, Juan Cruz  
- Vollenweider, Erich  

Universidad Nacional de Río Cuarto - Taller de Diseño de Software

---

## 📁 Estructura del Proyecto

```bash
proyecto-compiladores/
├── analisis-semantico/                  # Análisis semántico del compilador
│   ├── semantico.c                      # Implementación del recorrido del AST y validaciones semánticas
│   └── semantico.h                      # Declaración de funciones de análisis semántico
├── arbol-sintactico/                    # Estructura de árbol sintáctico abstracto (AST)
│   ├── arbol.c                          # Implementación de nodos, creación de árbol (binario/ternario), impresión y liberación
│   ├── arbol.h                          # Definición de estructuras (nodo, info, enums) y declaración de funciones
│   └── image_ast.c                      # Generación de archivos DOT y PNG para visualización gráfica del AST
├── assembler/                           # Estructura del codigo assembler
│   ├── assembler.c                      # Implementación del código ensamblador (x86-64 AT&T) a partir del código intermedio (tres direcciones).
│   └── assembler.h                      # Declaraciones públicas del generador assembly
├── codigo-intermedio/                   # Estructura del codigo intermedio (CI)
│   ├── codigo3dir.c                     # Implementación de constructores, impresores y utilidades para la lista de instrucciones de tres direcciones.
│   ├── codigo3dir.h                     # Definición de la estructura en memoria del código intermedio (3 direcciones).
│   ├── generador.c                      # Implementación de la generacion de instrucciones para hacer codigo intermedio
│   └── generador.h                      # Definición de la estructura en memoria de las intrucciones y declaración de funciones correspondientes
├── docs/                                # Documentación del proyecto
├── lexico_sintactico/                   # Análisis léxico y sintáctico
│   ├── lexer.l                          # Especificación Flex: definición de tokens y patrones léxicos
│   └── parser.y                         # Especificación Bison: gramática, reglas sintácticas y construcción del AST
├── runtime/                             # Implementaciones runtime para funciones extern declaradas en tests (.ctds).
│   └── func-extern.c                    # Contiene utilidades I/O usadas por los tests: print_int, print_bool, get_int, etc.
├── tabla-simbolos/                      # Gestión de tabla de símbolos con scopes anidados
│   ├── tabla_simbolos.c                 # Implementación: inicialización, inserción, búsqueda, apertura/cierre de scopes
│   └── tabla_simbolos.h                 # Definición de estructuras (scope, simbolo, tabla_simbolos) y declaración de funciones
├── tests/                               # Casos de prueba positivos y negativos para el compilador
│   ├── tests-assembler/                 # Carpeta con tests positivos para correr el código assembler generado
│   ├── tests-interactivo/               # Carpeta con tests interactivos para correr el código assembler generado
│   ├── tests-semantico/                 # Carpeta con tests semanticos positivos y negativos
│   └── tests-sintactico/                # Carpeta con tests sintacticos positivos y negativos
├── utils/                               # Funciones auxiliares
│   ├── calcular_tipo_expresion.c        # Cálculo de tipo de retorno de expresiones
│   ├── manejo_errores.c                 # Sistema de registro y reporte de errores semánticos
│   ├── manejo_errores.h                 # Declaración de funciones de manejo de errores
│   ├── verificar_asignacion_metodo.c    # Validación de tipos en asignaciones y métodos
│   └── verificar_parametros.c           # Verificación de cantidad y tipo de parámetros en llamadas
├── .expected                            # Es un script auxiliar independiente que muestra un informe del test seleccionado
├── main.c                               # Punto de entrada: parseo de argumentos, invocación de fases del compilador
└── Makefile                             # Automatización de compilación, ejecución de tests y limpieza
```

<br><br>

## 🛠️ Compilación y Ejecución

### Compilar el proyecto
```bash
make
```

### Ejecutar con test por defecto (test1 semantico)
```bash
make run
```

### Ejecutar un test específico
```bash
# Casos positivos (deben pasar)
make run TEST=tests/tests-sintactico/test1.ctds

# Casos negativos (deben fallar intencionalmente)
make run TEST=tests/tests-semantico/testneg1.ctds
```

### Limpiar archivos generados
```bash
make clean
```

### Ejecutar bloques de test especifico
```bash
make test-sintactico
```
> Esta regla recorre todos los tests sintacticos donde los positivos funcionan y negativos no funcionan (anda correctamente), donde si hago make test-all algunos no funcionan de los positivos ya que ve sintacticamente y semanticamente

```bash
make test-semantico
```
> Esta regla recorre todos los tests semanticos.

```bash
make test-assembler
```
> Esta regla es nueva y recorre todos los tests para el código assembler.

### Ejecutar tests con expected
```bash
chmod +x .expected
./.expected tests/tests-assembler/test1asm.ctds
```
> Esta regla es nueva y muestra un informe del test seleccionado.

### Ejecutar todos los test
```bash
make test-all
```
> Esta regla recorre todos los archivos dentro de la carpeta tests/, es decir ambas carpetas tests-semantico y tests-sintactico, ejecuta el compilador sobre cada uno de ellos y muestra en pantalla un reporte de cuales pasaron y cuales no.
> Estos comandos (`make`, `make run`, `make clean`) siguen funcionando como en la entrega anterior.

---

<br><br>

### Opciones Disponibles

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `-o <salida>` | Especifica el archivo de salida | `-o mi_programa` |
| `-target <etapa>` | Define hasta qué etapa compilar | `-target sem` |
| `-opt [optimizacion]` | Lista de optimizaciones (futuro) | `-opt O2` |
| `-debug` | Activa información de depuración | `-debug` |

⚠️ Nota: -opt no está implementado y se ignora. -debug todavia no funciona correctamente

⚠️ Nota: Si se quiere se puede ejecutar el debug de c, haciendo gdb ./c-tds 

## Etapas de Compilación (Target)

### 1. **lex** - Análisis Léxico
- **Propósito**: Tokenización del código fuente
- **Salida**: Secuencia de tokens
- **Implementación**: Ejecuta solo el lexer (`yylex()`) en bucle

### 2. **parse** - Análisis Sintáctico
- **Propósito**: 
    - Construcción del Árbol Sintáctico Abstracto (AST).
    - Insercion de simbolos en la Tabla de simbolos

- **Salida**: 
  - Representación textual del árbol
  - Archivo de imagen para visualización gráfica
- **Implementación**: Ejecuta parser (`yyparse()`) y genera visualización

### 3. **sem** - Análisis Semántico 
- **Propósito**: Verificación de tipos y reglas semánticas
- **Salida**: AST + TS + verificaciones semánticas

### 4. **codinter** - Generación Código Intermedio *(Etapa por defecto si solo se pone -target)*

- **Propósito**: Traducción del AST a código intermedio de tres direcciones.
- **Salida**: Archivo .txt con la generación de código intermedio.

### 5. **assembly** - Generación Código Assembly
- **Propósito**: Traducción del código intermedio a assembly x86-64
- **Salida**: Archivo .s con código assembly ejecutable
- **Arquitectura**: x86-64 (64 bits) siguiendo ABI System V
- **Características**:
  - Stack frame estándar con prólogo/epílogo
  - Convención de llamada Linux: parámetros en %rdi, %rsi, %rdx, %rcx, %r8, %r9
  - Valor de retorno en %rax
  - Variables locales en stack (offsets negativos desde %rbp)
  - Temporales mapeados a registros %r10 y %r11
- **Operaciones soportadas**:
  - Aritméticas: ADD, SUB, MUL, DIV, MOD, NEG
  - Lógicas: AND, OR, NOT
  - Comparaciones: EQ, LT, GT (genera valores booleanos 0/1)
  - Control de flujo: IF, IF-ELSE, WHILE con saltos condicionales
  - Llamadas a funciones: CALL con paso de parámetros y retorno


### Generación de Salida

#### Archivos Generados
- **AST textual**: Salida por consola
- **Tabla de simbolos textual**: Salida por consola
- **Archivo de imagen**: Para visualización gráfica del árbol
- **Nombre por defecto**: `ctds_arbol` si no se especifica `-o`
- **Código intermedio**: Archivo .txt con la generación del código intermedio.
- **Código assembly**: Archivo .s con la generación del código assembly a partir del código intermedio.

## Manejo de Errores

### Colores en Terminal
- 🔴 **Rojo** (`\033[31m`): Errores críticos que detienen la compilación
- 🟡 **Amarillo** (`\033[33m`): Advertencias y opciones desconocidas  
- 🟢 **Verde** (`\033[32m`): Compilación exitosa

## Ejemplos de Uso

```bash
# Análisis léxico únicamente
./c-tds -target lex programa.ctds

# Análisis sintáctico
./c-tds programa.ctds

# Análisis semántico
./c-tds -target sem programa.ctds

# Generación de código intermedio
./c-tds -target codinter programa.ctds

# Generación de código assembly (por defecto)
./c-tds -target assembly programa.ctds -o output.s

- Compilar y ejecutar:
  gcc output.s -o programa
  ./programa

```
