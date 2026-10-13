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
    for (const auto& diagnostico : diagnosticos) {
        if (diagnostico.severidad == SeveridadDiagnostico::Error) {
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
    std::stringstream flujoSalida;
    flujoSalida << "=== Resultado de Traduccion ===\n";
    flujoSalida << "Estado: ";
    if (esExitoso()) {
        flujoSalida << "EXITOSO";
    } else {
        flujoSalida << "CON ERRORES";
    }
    flujoSalida << "\n\n";

    if (!codigoGenerado.empty()) {
        flujoSalida << "[Codigo AArch64 Generado]:\n" << codigoGenerado << "\n";
    }

    if (!mapeos.empty()) {
        flujoSalida << "[Mapeos Didacticos (" << mapeos.size() << ")]:\n";
        for (const auto& mapeo : mapeos) {
            flujoSalida << "  Linea x86 " << mapeo.lineaOrigenX86 << " -> Regla [" << mapeo.reglaId << "]: "
               << mapeo.explicacionCorta << "\n";
            if (!mapeo.codigoOrigen.empty() && !mapeo.codigoDestino.empty()) {
                flujoSalida << "    Origen:  \"" << mapeo.codigoOrigen << "\"\n";
                flujoSalida << "    Destino: \"" << mapeo.codigoDestino << "\"\n";
            }
        }
        flujoSalida << "\n";
    }

    if (!diagnosticos.empty()) {
        flujoSalida << "[Diagnosticos (" << diagnosticos.size() << ")]:\n";
        for (const auto& diagnostico : diagnosticos) {
            flujoSalida << "  " << diagnostico.formatear() << "\n";
        }
    }

    return flujoSalida.str();
}

std::string ResultadoTraduccion::volcadoIR() const {
    std::stringstream flujoSalida;
    flujoSalida << "=== Volcado de Representacion Intermedia (IR) ===\n\n";

    flujoSalida << "[IR x86-64 de Entrada (AST)]:\n";
    if (irOrigen.empty()) {
        flujoSalida << "  (sin instrucciones de entrada registradas)\n";
    } else {
        for (const auto& instruccionOrigen : irOrigen) {
            flujoSalida << "  Linea " << instruccionOrigen.numeroLinea << ": ";
            if (instruccionOrigen.esEtiquetaPura()) {
                flujoSalida << "[ETIQUETA] " << instruccionOrigen.etiqueta << ":\n";
            } else {
                flujoSalida << "[" << codigoOperacionATexto(instruccionOrigen.codigoOperacion) << "]";
                if (instruccionOrigen.tieneEtiqueta()) {
                    flujoSalida << " (Etiqueta: " << instruccionOrigen.etiqueta << ")";
                }
                if (!instruccionOrigen.operandos.empty()) {
                    flujoSalida << " Operandos: ";
                    for (size_t indiceOperando = 0; indiceOperando < instruccionOrigen.operandos.size(); ++indiceOperando) {
                        if (indiceOperando > 0) {
                            flujoSalida << ", ";
                        }
                        flujoSalida << "[" << tipoOperandoATexto(instruccionOrigen.operandos[indiceOperando].tipo) << ": " << instruccionOrigen.operandos[indiceOperando].valor << "]";
                    }
                }
                flujoSalida << "\n";
            }
        }
    }

    flujoSalida << "\n[IR AArch64 de Destino]:\n";
    if (irDestino.empty()) {
        flujoSalida << "  (sin instrucciones de destino emitidas)\n";
    } else {
        for (const auto& instruccionDestino : irDestino) {
            flujoSalida << "  Linea " << instruccionDestino.lineaOrigen << ": ";
            if (instruccionDestino.esEtiquetaPura()) {
                flujoSalida << "[ETIQUETA] " << instruccionDestino.etiqueta << ":\n";
            } else {
                flujoSalida << "[" << codigoOperacionAArch64ATexto(instruccionDestino.codigoOperacion) << "] "
                   << instruccionDestino.emitirTexto();
                if (!instruccionDestino.reglaId.empty()) {
                    flujoSalida << " (Regla: " << instruccionDestino.reglaId << ")";
                }
                flujoSalida << "\n";
            }
        }
    }

    return flujoSalida.str();
}

std::string ResultadoTraduccion::explicacionesFormateadas() const {
    std::stringstream flujoSalida;
    flujoSalida << "=== Explicaciones Didacticas de Traduccion ===\n";
    if (mapeos.empty()) {
        flujoSalida << "(No hay mapeos didacticos disponibles)\n";
        return flujoSalida.str();
    }

    for (const auto& mapeo : mapeos) {
        flujoSalida << "Linea x86 " << mapeo.lineaOrigenX86 << " -> Regla [" << mapeo.reglaId << "]: "
           << mapeo.explicacionCorta << "\n";
        if (!mapeo.codigoOrigen.empty() && !mapeo.codigoDestino.empty()) {
            flujoSalida << "  x86-64:  \"" << mapeo.codigoOrigen << "\"\n";
            flujoSalida << "  AArch64: \"" << mapeo.codigoDestino << "\"\n";
        }
    }

    return flujoSalida.str();
}

