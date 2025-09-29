# Proyecto Compiladores - TDS25 🔍
## Árbol y Tabla de Símbolos

Esta rama contiene la segunda entrega del proyecto, implementando el árbol sintáctico, tabla de símbolos
y análisis semántico para el lenguaje **TDS25**.

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
├── docs/                                # Documentación del proyecto
├── lexico_sintactico/                   # Análisis léxico y sintáctico
│   ├── lexer.l                          # Especificación Flex: definición de tokens y patrones léxicos
│   └── parser.y                         # Especificación Bison: gramática, reglas sintácticas y construcción del AST
├── tabla-simbolos/                      # Gestión de tabla de símbolos con scopes anidados
│   ├── tabla_simbolos.c                 # Implementación: inicialización, inserción, búsqueda, apertura/cierre de scopes
│   └── tabla_simbolos.h                 # Definición de estructuras (scope, simbolo, tabla_simbolos) y declaración de funciones
├── tests/                               # Casos de prueba positivos y negativos para el compilador
├── utils/                               # Funciones auxiliares
│   ├── calcular_tipo_expresion.c        # Cálculo de tipo de retorno de expresiones
│   ├── manejo_errores.c                 # Sistema de registro y reporte de errores semánticos
│   ├── manejo_errores.h                 # Declaración de funciones de manejo de errores
│   ├── verificar_asignacion_metodo.c    # Validación de tipos en asignaciones y métodos
│   └── verificar_parametros.c           # Verificación de cantidad y tipo de parámetros en llamadas
├── main.c                               # Punto de entrada: parseo de argumentos, invocación de fases del compilador
└── Makefile                             # Automatización de compilación, ejecución de tests y limpieza
```

<br><br>

## 🛠️ Compilación y Ejecución

### Compilar el proyecto
```bash
make
```

### Ejecutar con test por defecto
```bash
make run
```

### Ejecutar un test específico
```bash
# Casos positivos (deben pasar)
make run TEST=tests/test1.ctds

# Casos negativos (deben fallar intencionalmente)
make run TEST=tests/testneg1.ctds
```

### Limpiar archivos generados
```bash
make clean
```
### Ejecutar todos los test
```bash
make test-all
```
:nota: Esta regla es nueva y recorre todos los archivos dentro de la carpeta tests/, ejecuta el compilador sobre cada uno de ellos y muestra en pantalla un reporte de cuales pasaron y cuales no.

✅ **Nota**: Estos comandos (`make`, `make run`, `make clean`, `make run TEST=...`) siguen funcionando como en la entrega anterior.

---

<br><br>

# 🚀 **Nueva Característica: Ejecutable con Flags**

Además del flujo anterior basado en `make`, ahora se incluye un ejecutable llamado **`c-tds`** que permite compilar usando diferentes opciones y etapas del compilador.

## Compilador C-TDS - Arquitectura y Flujo de Ejecución

## Descripción General

El compilador **C-TDS** es un compilador modular que permite compilar archivos con extensión `.ctds` a través de diferentes etapas de compilación, desde análisis léxico hasta generación de código assembly.

## Estructura del Comando

```bash
c-tds [opciones] archivo.ctds
```

### Opciones Disponibles

| Opción | Descripción | Ejemplo |
|--------|-------------|---------|
| `-o <salida>` | Especifica el archivo de salida | `-o mi_programa` |
| `-target <etapa>` | Define hasta qué etapa compilar | `-target parse` |
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

### 3. **sem** - Análisis Semántico *(Etapa por defecto si solo se pone -target)*
- **Propósito**: Verificación de tipos y reglas semánticas
- **Salida**: AST + TS + verificaciones semánticas

### 4. **codinter** - Generación Código Intermedio
- **Estado**: Pendiente implementación

### 5. **assembly** - Generación Código Assembly
- **Estado**: Pendiente implementación

### Generación de Salida

#### Archivos Generados
- **AST textual**: Salida por consola
- **Tabla de simbolos textual**: Salida por consola
- **Archivo de imagen**: Para visualización gráfica del árbol
- **Nombre por defecto**: `ctds_arbol` si no se especifica `-o`

## Manejo de Errores

### Colores en Terminal
- 🔴 **Rojo** (`\033[31m`): Errores críticos que detienen la compilación
- 🟡 **Amarillo** (`\033[33m`): Advertencias y opciones desconocidas  
- 🟢 **Verde** (`\033[32m`): Compilación exitosa

## Ejemplos de Uso

```bash
# Análisis léxico únicamente
./c-tds -target lex programa.ctds

# Análisis sintáctico (por defecto)
./c-tds programa.ctds

```



## Estado de Implementación

| Etapa | Estado | Funcionalidad | Próximos Pasos |
|-------|--------|---------------|----------------|
| **Análisis Léxico** | ✅ **Completo** | Tokenización funcional | - |
| **Análisis Sintáctico** | ✅ **Completo** | AST + visualización | - |
| **Análisis Semántico** | ✅ **Completo** | Estructura básica | - |
| **Código Intermedio** | ❌ **Pendiente** | No implementado |
| **Assembly** | ❌ **Pendiente** | No implementado |
