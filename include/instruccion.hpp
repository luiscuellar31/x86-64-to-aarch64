#ifndef INSTRUCCION_HPP
#define INSTRUCCION_HPP

#include <string>
#include <vector>
#include <iostream>
#include <cstdint>

/**
 * @brief Códigos de operación x86-64 soportados inicialmente.
 */
enum class CodigoOperacion {
    Desconocido,
    Etiqueta, // Representa una definicion de etiqueta pura (ej. "inicio:")
    Mov,
    Add,
    Sub,
    And,
    Or,
    Xor,
    Cmp,
    Jmp,
    Je,
    Jne,
    Jl,
    Jle,
    Jg,
    Jge,
    Ret
};

/**
 * @brief Convierte un CodigoOperacion a su representación textual mnemónica.
 */
std::string codigoOperacionATexto(CodigoOperacion codigo);

/**
 * @brief Obtiene el CodigoOperacion a partir de una cadena de mnemónico.
 */
CodigoOperacion textoACodigoOperacion(const std::string& texto);

/**
 * @brief Tipos de operandos que puede recibir una instrucción.
 */
enum class TipoOperando {
    Desconocido,
    Registro,
    Inmediato,
    Etiqueta
};

/**
 * @brief Convierte un TipoOperando a una descripción en texto.
 */
std::string tipoOperandoATexto(TipoOperando tipo);

/**
 * @brief Representa un operando individual dentro de una instrucción x86-64 con datos semanticos.
 */
struct Operando {
    TipoOperando tipo;
    std::string valor;
    int64_t valorNumerico; // Valor parsed si es inmediato numerico

    // Constructor por defecto
    Operando();

    // Constructor con tipo y valor
    Operando(TipoOperando tipoOperando, const std::string& valorTexto, int64_t valorNum = 0);

    // Metodos semanticos de consulta
    bool esRegistro() const;
    bool esInmediato() const;
    bool esEtiqueta() const;

    // Métodos auxiliares para crear operandos de forma clara
    static Operando crearRegistro(const std::string& nombreRegistro);
    static Operando crearInmediato(const std::string& valorInmediato);
    static Operando crearInmediato(int64_t valorNum);
    static Operando crearEtiqueta(const std::string& nombreEtiqueta);

    // Representación textual para depuración o explicaciones didácticas
    std::string aTexto() const;
};

/**
 * @brief Representa una instrucción ensamblador x86-64 completa en memoria.
 */
struct Instruccion {
    CodigoOperacion codigoOperacion;
    std::vector<Operando> operandos;
    int numeroLinea;
    std::string etiqueta; // Nombre de etiqueta asociada o definida en esta linea

    // Constructor con código de operación y número de línea
    Instruccion(CodigoOperacion codigo, int linea = 1);

    // Constructor con código, lista de operandos y número de línea
    Instruccion(CodigoOperacion codigo, const std::vector<Operando>& listaOperandos, int linea = 1);

    // Métodos de consulta semantica
    bool tieneEtiqueta() const;
    bool esEtiquetaPura() const;
    bool esInstruccion(CodigoOperacion codigo) const;
    bool esSalto() const;
    bool esSaltoCondicional() const;
    size_t cantidadOperandos() const;
    const Operando& operando(size_t indice) const;

    // Permite agregar un operando paso a paso
    void agregarOperando(const Operando& operando);

    // Imprime la estructura jerárquica de la instrucción de manera legible
    void imprimir(std::ostream& salida = std::cout) const;
};

#endif // INSTRUCCION_HPP
