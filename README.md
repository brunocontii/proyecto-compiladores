# Proyecto Compiladores - TDS25 🔍

## Análizador Léxico y Sintáctico

Esta rama contiene la **primera entrega** del proyecto, implementando el analizador léxico (lexer) y sintáctico (parser) para el lenguaje TDS25.

### 👥 Equipo de Desarrollo
- **Conti, Bruno**
- **Gonzalez, Juan Cruz**  
- **Vollenweider, Erich**

*Universidad Nacional de Río Cuarto - Taller de Diseño de Software*

---

## 📁 Estructura del Proyecto

```
proyecto-compiladores/
├── docs/                   # Carpeta con las consignas y entregas
├── lexico_sintactico/      # Carpeta con los archivos Flex y Bison
│   ├── lexer.l             # Analizador léxico (Flex)
│   ├── parser.y            # Analizador sintáctico (Bison)
├── tests/                  # Casos de prueba
└── Makefile
```

---

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

# Casos negativos (deben fallar intencionalmente)
make run TEST=tests/testneg1.ctds
make run TEST=tests/testneg2.ctds
make run TEST=tests/testneg3.ctds
make run TEST=tests/testneg4.ctds
```

### Limpiar archivos generados
```bash
make clean
```

---

## 🔤 Análisis Léxico (Lexer)

El analizador léxico reconoce los siguientes elementos del lenguaje TDS25:

### Palabras Reservadas
`program`, `integer`, `bool`, `void`, `extern`, `return`, `if`, `else`, `then`, `while`, `true`, `false`

### Operadores
- **Aritméticos**: `+`, `-`, `*`, `/`, `%`
- **Lógicos**: `&&`, `||`, `!`
- **Comparación**: `=`, `==`, `<`, `>`

### Símbolos
- **Delimitadores**: `(`, `)`, `{`, `}`, `;`, `,`
- **Identificadores**: Letras, números y guión bajo (iniciando con letra)
- **Constantes**: Enteros positivos

### Características Especiales
- ✅ Ignora espacios, tabulaciones y saltos de línea
- ✅ Maneja comentarios de línea (`//`) y de bloque (`/* ... */`)
- ✅ Imprime cada token reconocido para depuración
- ✅ Detecta y reporta caracteres no válidos

---

## 📝 Análisis Sintáctico (Parser)

El parser valida la estructura del programa según la gramática TDS25:

### Estructura del Programa
```
program {
    // Primero: declaraciones de variables
    int variable = 5;
    
    // Después: declaraciones de métodos
    void metodo() {
        // cuerpo del método
    }
}
```

### Características Implementadas
- ✅ Precedencia y asociatividad correcta de operadores
- ✅ Declaraciones de variables con asignación opcional
- ✅ Declaraciones de métodos con parámetros y tipos de retorno
- ✅ Soporte para funciones externas (`extern`)
- ✅ Estructuras de control (`if/then/else`, `while`)
- ✅ Expresiones complejas (aritméticas, lógicas, comparación)
- ✅ Bloques anidados con variables locales
- ✅ Manejo de errores sintácticos con número de línea

---

## 🧪 Casos de Prueba

### Tests Positivos ✅
Los siguientes archivos contienen programas válidos que el compilador debe procesar correctamente:
- `test1.ctds`, `test2.ctds`, `test3.ctds`

### Tests Negativos 🔴
Estos casos están diseñados para fallar y validar la detección de errores:

| Test | Error Detectado |
|------|-----------------|
| `testneg1.ctds` | Variable declarada después de un método |
| `testneg2.ctds` | Falta la declaración obligatoria `program` |
| `testneg3.ctds` | Método sin bloque ni palabra clave `extern` |
| `testneg4.ctds` | Sentencias antes de declaraciones en un bloque |
