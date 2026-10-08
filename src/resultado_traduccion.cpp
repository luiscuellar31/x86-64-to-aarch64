#include "resultado_traduccion.hpp"
#include <sstream>

MapeoTraduccion::MapeoTraduccion()
    : lineaOrigenX86(1), reglaId(""), explicacionCorta(""), codigoOrigen(""), codigoDestino("") {
}

MapeoTraduccion::MapeoTraduccion(int lineaX86, const std::string& regla, const std::string& explicacion)
    : lineaOrigenX86(lineaX86), reglaId(regla), explicacionCorta(explicacion),
      codigoOrigen(""), codigoDestino("") {
}

ResultadoTraduccion::ResultadoTraduccion()
    : exito(true), codigoGenerado("") {
}

bool ResultadoTraduccion::esExitoso() const {
    return exito && !tieneErrores();
}

bool ResultadoTraduccion::tieneErrores() const {
    for (const auto& diag : diagnosticos) {
        if (diag.severidad == SeveridadDiagnostico::Error) {
            return true;
        }
    }
    return false;
}

void ResultadoTraduccion::agregarDiagnostico(const Diagnostico& diagnostico) {
    diagnosticos.push_back(diagnostico);
    if (diagnostico.severidad == SeveridadDiagnostico::Error) {
        exito = false;
    }
}

void ResultadoTraduccion::agregarMapeo(const MapeoTraduccion& mapeo) {
    mapeos.push_back(mapeo);
}

const MapeoTraduccion* ResultadoTraduccion::buscarMapeoPorLineaX86(int lineaX86) const {
    for (const auto& mapeo : mapeos) {
        if (mapeo.lineaOrigenX86 == lineaX86) {
            return &mapeo;
        }
    }
    return nullptr;
}

std::string ResultadoTraduccion::resumenFormateado() const {
    std::stringstream ss;
    ss << "=== Resultado de Traduccion ===\n";
    ss << "Estado: " << (esExitoso() ? "EXITOSO" : "CON ERRORES") << "\n\n";

    if (!codigoGenerado.empty()) {
        ss << "[Codigo AArch64 Generado]:\n" << codigoGenerado << "\n";
    }

    if (!mapeos.empty()) {
        ss << "[Mapeos Didacticos (" << mapeos.size() << ")]:\n";
        for (const auto& m : mapeos) {
            ss << "  Linea x86 " << m.lineaOrigenX86 << " -> Regla [" << m.reglaId << "]: "
               << m.explicacionCorta << "\n";
            if (!m.codigoOrigen.empty() && !m.codigoDestino.empty()) {
                ss << "    Origen:  \"" << m.codigoOrigen << "\"\n";
                ss << "    Destino: \"" << m.codigoDestino << "\"\n";
            }
        }
        ss << "\n";
    }

    if (!diagnosticos.empty()) {
        ss << "[Diagnosticos (" << diagnosticos.size() << ")]:\n";
        for (const auto& diag : diagnosticos) {
            ss << "  " << diag.formatear() << "\n";
        }
    }

    return ss.str();
}
