#ifndef ANALIZADOR_SINTACTICO_HPP
#define ANALIZADOR_SINTACTICO_HPP

#include "instruccion.hpp"
#include "normalizador.hpp"
#include "analizador_lexico.hpp"
#include <string>
#include <vector>

/**
 * @brief Analizador sintáctico lineal para ensamblador x86-64.
 *
 * Responsabilidades:
 * - Consumir tokens provenientes del AnalizadorLexico.
 * - Identificar mnemónicos (mov, add, sub, ret).
 * - Identificar operandos (registros, inmediatos, etiquetas).
 * - Procesar etiquetas en su propia línea o precediendo instrucciones.
 * - Construir instancias de Instruccion listas para validación y traducción.
 */
class AnalizadorSintactico {
public:
    /**
     * @brief Analiza una secuencia de tokens y genera una Instruccion.
     * @param tokens Secuencia de tokens correspondiente a una linea o sentencia.
     * @return Estructura Instruccion construida.
     */
    static Instruccion analizarTokens(const std::vector<Token>& tokens);

    /**
     * @brief Analiza una linea ya normalizada y construye su representacion Instruccion.
     * @param linea Linea previamente procesada por el Normalizador.
     * @return Estructura Instruccion correspondiente.
     */
    static Instruccion analizarLinea(const LineaNormalizada& linea);

    /**
     * @brief Normaliza y analiza una linea de texto crudo.
     * @param textoLinea Texto de la linea en ensamblador.
     * @param numeroLinea Numero de linea de origen (por defecto 1).
     * @return Estructura Instruccion construida.
     */
    static Instruccion analizarTexto(const std::string& textoLinea, int numeroLinea = 1);

    /**
     * @brief Analiza un programa completo en codigo fuente y genera la lista de instrucciones.
     * @param codigoFuente Texto con el programa completo.
     * @return Vector de instrucciones identificadas en orden de aparicion.
     */
    static std::vector<Instruccion> analizarPrograma(const std::string& codigoFuente);

    /**
     * @brief Clasifica un texto de operando individual (registro, inmediato o etiqueta).
     * @param textoOperando Texto del operando sin espacios sobrantes.
     * @return Estructura Operando con su tipo y valor detectados.
     */
    static Operando clasificarOperando(const std::string& textoOperando);

    /**
     * @brief Analiza un operando de memoria estructurado delimitado por corchetes.
     * @param tokens Vector de tokens de la instruccion.
     * @param indice Indice actual dentro del vector (se actualiza al avanzar los tokens consumidos).
     * @return Operando estructurado de tipo Memoria o Desconocido si hay error sintactico.
     */
    static Operando analizarOperandoMemoria(const std::vector<Token>& tokens, size_t& indice);
};

#endif // ANALIZADOR_SINTACTICO_HPP
