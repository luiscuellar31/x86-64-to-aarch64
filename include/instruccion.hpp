#ifndef INSTRUCCION_HPP
#define INSTRUCCION_HPP

#include <string>
#include <vector>
#include <iostream>

/**
 * @brief Códigos de operación x86-64 soportados inicialmente.
 */
enum class CodigoOperacion {
    Desconocido,
    Mov,
    Add,
    Sub,
    Ret
};

/**
 * @brief Convierte un CodigoOperacion a su representación textual mnemónica.
 */
std::string codigoOperacionATexto(CodigoOperacion codigo);

/**
 * @brief Tipos de operandos que puede recibir una instrucción.
 */
enum class TipoOperando {
    Desconocido,
    Registro,
    Inmediato
};

/**
 * @brief Convierte un TipoOperando a una descripción en texto.
 */
std::string tipoOperandoATexto(TipoOperando tipo);

/**
 * @brief Representa un operando individual dentro de una instrucción.
 */
struct Operando {
    TipoOperando tipo;
    std::string valor;

    // Constructor por defecto
    Operando();

    // Constructor con tipo y valor
    Operando(TipoOperando tipoOperando, const std::string& valorTexto);

    // Métodos auxiliares para crear operandos de forma clara
    static Operando crearRegistro(const std::string& nombreRegistro);
    static Operando crearInmediato(const std::string& valorInmediato);

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

    // Constructor con código de operación y número de línea
    Instruccion(CodigoOperacion codigo, int linea = 1);

    // Constructor con código, lista de operandos y número de línea
    Instruccion(CodigoOperacion codigo, const std::vector<Operando>& listaOperandos, int linea = 1);

    // Permite agregar un operando paso a paso
    void agregarOperando(const Operando& operando);

    // Imprime la estructura jerárquica de la instrucción de manera legible
    void imprimir(std::ostream& salida = std::cout) const;
};

#endif // INSTRUCCION_HPP
