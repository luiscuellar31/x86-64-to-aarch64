#ifndef TRADUCTOR_HPP
#define TRADUCTOR_HPP

#include "instruccion.hpp"
#include "ir_aarch64.hpp"
#include "resultado_traduccion.hpp"
#include <string>
#include <vector>

/**
 * @brief Motor principal de traducción determinista de ensamblador x86-64 a AArch64.
 *
 * Aplica el pipeline completo:
 * 1. Normalización
 * 2. Análisis sintáctico
 * 3. Validación semántica (Gate V0)
 * 4. Mapeo determinista de registros y reglas semánticas
 * 5. Emisión de código AArch64 con metadatos explicativos
 */
class Traductor {
public:
    /**
     * @brief Traduce un programa completo en código fuente textual x86-64 a AArch64.
     * @param codigoFuente Texto del programa x86-64.
     * @return Estructura ResultadoTraduccion con el código emitido, mapeos y diagnósticos.
     */
    static ResultadoTraduccion traducir(const std::string& codigoFuente);

    /**
     * @brief Traduce una lista de instrucciones ya analizadas.
     * @param instrucciones Vector de instrucciones x86-64.
     * @return ResultadoTraduccion estructurado.
     */
    static ResultadoTraduccion traducirInstrucciones(const std::vector<Instruccion>& instrucciones);

    /**
     * @brief Traduce una sola instrucción válida a su instrucción IR equivalente en AArch64.
     * @param instruccion Instrucción x86-64 validada.
     * @param mapeoSalida Mapeo pedagógico donde se registran reglaId y explicación didáctica.
     * @return Instrucción AArch64 equivalente.
     */
    static InstruccionAArch64 traducirInstruccion(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida);
};

#endif // TRADUCTOR_HPP
