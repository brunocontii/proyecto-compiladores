# Proyecto Compiladores - TDS25 🔍
## Árbol y Tabla de Símbolos

Esta rama contiene la segunda entrega del proyecto, implementando el árbol sintáctico y la tabla de símbolos para el lenguaje **TDS25**.

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
├── docs/                   # Carpeta con las consignas y entregas
├── lexico_sintactico/      # Carpeta con los archivos Flex y Bison
├── arbol-sintactico/       # Estructura de árbol
│   ├── arbol.h             # Definición de la estructura (nodos)
│   ├── arbol.c             # Creación de árbol (binario y ternario) impresión en consola y liberación
│   ├── image_ast.c         # Representación gráfica del árbol creado
├── tabla-simbolos/         # Estructura de la tabla de símbolos
│   ├── tabla_simbolos.h    # Definición de la estructura y alcance
│   ├── tabla_simbolos.c    # Inicialización, apertura/cierre de alcance, inserción, búsqueda y visualización
├── tests/                  # Casos de prueba nuevos
└── Makefile
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
make run TEST=tests/test2.ctds
make run TEST=tests/test3.ctds
make run TEST=tests/test4.ctds # nuevo

# Casos negativos (deben fallar intencionalmente)
make run TEST=tests/testneg1.ctds
make run TEST=tests/testneg2.ctds
make run TEST=tests/testneg3.ctds
make run TEST=tests/testneg4.ctds
make run TEST=tests/testneg5.ctds # nuevo
```

### Limpiar archivos generados
```bash
make clean
```
### Ejecutar todos los test
```bash
make test-all
```
📝 Esta regla es nueva y recorre todos los archivos dentro de la carpeta tests/, ejecuta el compilador sobre cada uno de ellos y muestra en pantalla un reporte de cuales pasaron y cuales no.

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

## Etapas de Compilación (Target)

### 1. **lex** - Análisis Léxico
- **Propósito**: Tokenización del código fuente
- **Salida**: Secuencia de tokens
- **Implementación**: Ejecuta solo el lexer (`yylex()`) en bucle

### 2. **parse** - Análisis Sintáctico *(Etapa por defecto si solo se pone -target)*
- **Propósito**: 
    - Construcción del Árbol Sintáctico Abstracto (AST).
    - Insercion de simbolos en la Tabla de simbolos

- **Salida**: 
  - Representación textual del árbol
  - Archivo de imagen para visualización gráfica
- **Implementación**: Ejecuta parser (`yyparse()`) y genera visualización

### 3. **sem** - Análisis Semántico
- **Propósito**: Verificación de tipos y reglas semánticas
- **Estado**: En desarrollo
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
| **Análisis Semántico** | 🟡 **En desarrollo** | Estructura básica | Verificación de tipos |
| **Código Intermedio** | ❌ **Pendiente** | No implementado |
| **Assembly** | ❌ **Pendiente** | No implementado |