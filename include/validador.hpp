#ifndef VALIDADOR_HPP
#define VALIDADOR_HPP

#include "instruccion.hpp"
#include "diagnostico.hpp"
#include <vector>

/**
 * @brief Módulo de validación semántica de instrucciones x86-64 para el subconjunto V0.
 *
 * Responsabilidades:
 * - Comprobar que el código de operación esté soportado en la versión activa.
 * - Validar que la cantidad de operandos sea la requerida (0 para ret, 2 para mov/add/sub).
 * - Verificar que el operando de destino sea un registro modificable y no un inmediato.
 * - Verificar que los registros pertenezcan al subconjunto soportado de 64 bits.
 * - Reportar diagnósticos estructurados con códigos (E001, E014, E021, E022, E023).
 */
class Validador {
public:
    /**
     * @brief Valida una instrucción individual contra las reglas del subconjunto.
     * @param instruccion Instrucción a validar.
     * @param gestor Gestor donde se acumulan los diagnósticos.
     * @return true si la instrucción es válida, false si contiene errores semánticos.
     */
    static bool validarInstruccion(const Instruccion& instruccion, GestorDiagnosticos& gestor);

    /**
     * @brief Valida un programa completo representado como lista de instrucciones.
     * @param instrucciones Vector de instrucciones.
     * @param gestor Gestor donde se acumulan los diagnósticos.
     * @return true si todo el programa es válido, false si existen errores.
     */
    static bool validarPrograma(const std::vector<Instruccion>& instrucciones, GestorDiagnosticos& gestor);
};

#endif // VALIDADOR_HPP
