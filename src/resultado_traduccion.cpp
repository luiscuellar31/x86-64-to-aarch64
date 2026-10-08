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

std::string ResultadoTraduccion::volcadoIR() const {
    std::stringstream ss;
    ss << "=== Volcado de Representacion Intermedia (IR) ===\n\n";

    ss << "[IR x86-64 de Entrada (AST)]:\n";
    if (irOrigen.empty()) {
        ss << "  (sin instrucciones de entrada registradas)\n";
    } else {
        for (const auto& inst : irOrigen) {
            ss << "  Linea " << inst.numeroLinea << ": ";
            if (inst.esEtiquetaPura()) {
                ss << "[ETIQUETA] " << inst.etiqueta << ":\n";
            } else {
                ss << "[" << codigoOperacionATexto(inst.codigoOperacion) << "]";
                if (inst.tieneEtiqueta()) {
                    ss << " (Etiqueta: " << inst.etiqueta << ")";
                }
                if (!inst.operandos.empty()) {
                    ss << " Operandos: ";
                    for (size_t i = 0; i < inst.operandos.size(); ++i) {
                        if (i > 0) {
                            ss << ", ";
                        }
                        ss << "[" << tipoOperandoATexto(inst.operandos[i].tipo) << ": " << inst.operandos[i].valor << "]";
                    }
                }
                ss << "\n";
            }
        }
    }

    ss << "\n[IR AArch64 de Destino]:\n";
    if (irDestino.empty()) {
        ss << "  (sin instrucciones de destino emitidas)\n";
    } else {
        for (const auto& aarchInst : irDestino) {
            ss << "  Linea " << aarchInst.lineaOrigen << ": ";
            if (aarchInst.esEtiquetaPura()) {
                ss << "[ETIQUETA] " << aarchInst.etiqueta << ":\n";
            } else {
                ss << "[" << codigoOperacionAArch64ATexto(aarchInst.codigoOperacion) << "] "
                   << aarchInst.emitirTexto();
                if (!aarchInst.reglaId.empty()) {
                    ss << " (Regla: " << aarchInst.reglaId << ")";
                }
                ss << "\n";
            }
        }
    }

    return ss.str();
}

std::string ResultadoTraduccion::explicacionesFormateadas() const {
    std::stringstream ss;
    ss << "=== Explicaciones Didacticas de Traduccion ===\n";
    if (mapeos.empty()) {
        ss << "(No hay mapeos didacticos disponibles)\n";
        return ss.str();
    }

    for (const auto& m : mapeos) {
        ss << "Linea x86 " << m.lineaOrigenX86 << " -> Regla [" << m.reglaId << "]: "
           << m.explicacionCorta << "\n";
        if (!m.codigoOrigen.empty() && !m.codigoDestino.empty()) {
            ss << "  x86-64:  \"" << m.codigoOrigen << "\"\n";
            ss << "  AArch64: \"" << m.codigoDestino << "\"\n";
        }
    }

    return ss.str();
}

