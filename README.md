# Proyecto Compiladores - TDS25 🔍
## Optimizaciones

Esta rama contiene la quinta entrega del proyecto, implementando optimizaciones

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
├── analisis-semantico/                  # Análisis semántico del compilador.
├── arbol-sintactico/                    # Estructura de árbol sintáctico abstracto (AST).
├── docs/                                # Documentación del proyecto.
├── lexico_sintactico/                   # Análisis léxico y sintáctico.
├── optimizaciones/                      # Estructura para las optimizaciones del código.
│   ├── codigo_muerto_bloque.c           # Recorre el árbol AST para eliminar bloques de código que nunca van a ser accedidos debido a la condición.
│   ├── codigo_muerto_bloque.h           # Definición de la estructura para eliminar bloques de código de las estructuras de control.
│   ├── codigo_muerto_var.c              # Recorre el árbol AST para marcar y eliminar las variables sin usos.
│   ├── codigo_muerto_var.h              # Definición de la estructura para eliminar las variables que no son usadas.
│   ├── codigo_muerto.c                  # Recorre el árbol AST para eliminar bloques de código muerto.
│   ├── codigo_muerto.h                  # Definición de la estructura para eliminar bloques de código inalcanzables.
│   ├── operaciones.c                    # Métodos para la optimizacióon de operaciones aritméticas.
│   ├── operaciones.h                    # Perfil de métodos públicos para las optimizaciones de las operaciones aritmeticas.
│   ├── optimizaciones.c                 # Método principal para aplicar todas las optimizaciones.
│   ├── optimizaciones.h                 # Perfil del método principal de aplicación de todas las optimizaciones.
│   ├── plegado_constantes.c             # Métodos para la optimización de plegado y propagación de valores literales.
│   └── plegado_constantes.h             # Perfil del método público para la optimización de plegado y propagación de valores literales.
├── runtime/                             # Implementaciones runtime para funciones extern declaradas en tests (.ctds).
├── tabla-simbolos/                      # Gestión de tabla de símbolos con scopes anidados.
├── tests/                               # Casos de prueba positivos y negativos para el compilador.
├── .expected                            # Es un script auxiliar independiente que muestra un informe del test seleccionado.
├── main.c                               # Punto de entrada: parseo de argumentos, invocación de fases del compilador.
└── Makefile                             # Automatización de compilación, ejecución de tests y limpieza.
```

<br><br>

## 🛠️ Compilación y Ejecución

### Compilar el proyecto
```bash
make
```

### Limpiar archivos generados
```bash
make clean
```

### Esta regla da una ayuda de todos los comandos disponibles
```bash
make help
```

### Ejecutar todos los test con optimizaciones
```bash
make test-assembler-opt TEST_OPT=var-muertas
make test-assembler-opt TEST_OPT=prop-constantes
make test-assembler-opt TEST_OPT=operaciones
make test-assembler-opt TEST_OPT=cod-inalcanzable
make test-assembler-opt TEST_OPT=cod-bloque
```
> Estas reglas recorren todos los archivos dentro de la carpeta tests/tests-assembler/ ejecutando la optimización correspondiente.

```bash
make test-assembler-opt TEST_OPT=all
```
> Esta regla recorre todos los archivos dentro de la carpeta tests/tests-assembler/ y ejecuta todas las optimizaciones juntas.

### Ejecutar todos los test con optimizaciones
```bash
make test-optimizacion-compare
make test-optimizacion-detalle
```
> Estas reglas recorren todos los archivos dentro de la carpeta tests/tests-optimizacion/ sin optimizaciones y con optimizaciones para comparar los resultados.

```bash
make run-all-opt TEST=<test>
```
> Esta regla ejecuta todas las optimizaciones sobre el test especificado

---

<br><br>

### Opciones Disponibles

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `-target <etapa>`    | Define hasta qué etapa compilar | `-target sem` |
| `-opt [optimizacion]`| Lista de optimizaciones (futuro)| `-opt O2` |

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

### 4. **codinter** - Generación Código Intermedio 

- **Propósito**: Traducción del AST a código intermedio de tres direcciones.
- **Salida**: Archivo .txt con la generación de código intermedio.

### 5. **assembly** - Generación Código Assembly *(Etapa por defecto si solo se pone -target)*
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

### 6. **optimizaciones** - Optimizaciones disponibles
- **Propósito**: Mejorar el rendimiento y reducir el tamaño del arbol y/o del código objeto, dependiendo la optimizacion pero sin alterar su comportamiento final.
- **Entrada**: Arbol AST y/o código objeto.
- **Salida**: Arbol podado y/o código objeto optimizado.
- **Implementación**: Módulo que aplica transformaciones locales y globales sobre el código objeto y/o el arbol AST antes/durante la traducción a assembly.

### Generación de Salida

#### Archivos Generados
- **AST textual**: Salida por consola
- **Tabla de simbolos textual**: Salida por consola
- **Archivo de imagen**: Para visualización gráfica del árbol
- **Código intermedio**: Salida por consola
- **Código assembly**: Archivo .s con la generación del código assembly a partir del código intermedio.
