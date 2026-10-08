#ifndef RESULTADO_TRADUCCION_HPP
#define RESULTADO_TRADUCCION_HPP

#include "diagnostico.hpp"
#include "instruccion.hpp"
#include "ir_aarch64.hpp"
#include <string>
#include <vector>

/**
 * @brief Vincula una instrucción o línea x86-64 con su salida en AArch64 y metadatos pedagógicos.
 */
struct MapeoTraduccion {
    int lineaOrigenX86;
    std::vector<int> lineasDestinoAArch64;
    std::string reglaId;          // Identificador formal de la regla (ej. "ARITH_ADD_IMM")
    std::string explicacionCorta; // Explicación didáctica del cambio arquitectónico
    std::string codigoOrigen;     // Texto x86-64 original
    std::string codigoDestino;    // Texto AArch64 resultante

    MapeoTraduccion();
    MapeoTraduccion(int lineaX86, const std::string& regla, const std::string& explicacion);
};

/**
 * @brief Estructura completa con el resultado estructurado de un proceso de traducción.
 *
 * Contiene:
 * - El código ensamblador emitido en AArch64.
 * - Lista estructurada de diagnósticos (errores y advertencias).
 * - Mapeos entre líneas de origen y destino con justificaciones didácticas.
 * - Representaciones intermedias de origen (AST x86-64) y destino (IR AArch64).
 */
struct ResultadoTraduccion {
    bool exito;
    std::string codigoGenerado;
    std::vector<Diagnostico> diagnosticos;
    std::vector<MapeoTraduccion> mapeos;
    std::vector<Instruccion> irOrigen;          // Representación intermedia AST de entrada
    std::vector<InstruccionAArch64> irDestino;   // Representación intermedia IR de salida

    ResultadoTraduccion();

    bool esExitoso() const;
    bool tieneErrores() const;

    void agregarDiagnostico(const Diagnostico& diagnostico);
    void agregarMapeo(const MapeoTraduccion& mapeo);

    // Permite consultar los metadatos didácticos asociados a una línea específica de x86-64
    const MapeoTraduccion* buscarMapeoPorLineaX86(int lineaX86) const;

    // Genera un resumen textual del resultado incluyendo diagnósticos y mapeos explicativos
    std::string resumenFormateado() const;

    // Genera un volcado textual estructurado de las representaciones intermedias (IR)
    std::string volcadoIR() const;

    // Genera un informe detallado con las explicaciones didácticas de las reglas aplicadas
    std::string explicacionesFormateadas() const;
};

#endif // RESULTADO_TRADUCCION_HPP
