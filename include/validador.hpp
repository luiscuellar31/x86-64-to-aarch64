#ifndef VALIDADOR_HPP
#define VALIDADOR_HPP

#include "instruccion.hpp"
#include "diagnostico.hpp"
#include <vector>

/**
 * @brief Módulo de validación semántica del subconjunto entero de 64 bits de x86-64.
 *
 * Responsabilidades:
 * - Comprobar que el código de operación esté soportado.
 * - Validar la cantidad y forma de los operandos según cada instrucción.
 * - Verificar registros y accesos a memoria local de 64 bits con base rbp/rsp.
 * - Rechazar usos de rsp que las reglas de emisión actuales no representan.
 * - Comprobar que las etiquetas referenciadas existan y no se definan más de una vez.
 * - Reportar diagnósticos estructurados con códigos y números de línea.
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
