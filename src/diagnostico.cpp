#include "diagnostico.hpp"

std::string severidadDiagnosticoATexto(SeveridadDiagnostico severidad) {
    switch (severidad) {
        case SeveridadDiagnostico::Error:
            return "ERROR";
        case SeveridadDiagnostico::Advertencia:
            return "ADVERTENCIA";
        case SeveridadDiagnostico::Informacion:
            return "INFO";
        default:
            return "DESCONOCIDO";
    }
}

Diagnostico::Diagnostico()
    : severidad(SeveridadDiagnostico::Informacion), codigo(""),
      numeroLinea(1), numeroColumna(0), mensaje("") {
}

Diagnostico::Diagnostico(SeveridadDiagnostico nivel, const std::string& codigoError,
                         int linea, const std::string& textoMensaje, int columna)
    : severidad(nivel), codigo(codigoError),
      numeroLinea(linea), numeroColumna(columna), mensaje(textoMensaje) {
}

Diagnostico Diagnostico::crearError(const std::string& codigoError, int linea,
                                    const std::string& mensaje, int columna) {
    return Diagnostico(SeveridadDiagnostico::Error, codigoError, linea, mensaje, columna);
}

Diagnostico Diagnostico::crearAdvertencia(const std::string& codigoError, int linea,
                                          const std::string& mensaje, int columna) {
    return Diagnostico(SeveridadDiagnostico::Advertencia, codigoError, linea, mensaje, columna);
}

Diagnostico Diagnostico::crearInformacion(const std::string& codigoError, int linea,
                                          const std::string& mensaje, int columna) {
    return Diagnostico(SeveridadDiagnostico::Informacion, codigoError, linea, mensaje, columna);
}

std::string Diagnostico::formatear() const {
    std::string encabezado = "linea " + std::to_string(numeroLinea);
    if (numeroColumna > 0) {
        encabezado += ":" + std::to_string(numeroColumna);
    }

    std::string resultado = encabezado + ": [" + codigo + "] " +
                            severidadDiagnosticoATexto(severidad) + ": " + mensaje;
    return resultado;
}

GestorDiagnosticos::GestorDiagnosticos() {
}

void GestorDiagnosticos::agregar(const Diagnostico& diagnostico) {
    listaDiagnosticos.push_back(diagnostico);
}

void GestorDiagnosticos::agregarError(const std::string& codigo, int linea,
                                      const std::string& mensaje, int columna) {
    listaDiagnosticos.push_back(Diagnostico::crearError(codigo, linea, mensaje, columna));
}

void GestorDiagnosticos::agregarAdvertencia(const std::string& codigo, int linea,
                                            const std::string& mensaje, int columna) {
    listaDiagnosticos.push_back(Diagnostico::crearAdvertencia(codigo, linea, mensaje, columna));
}

void GestorDiagnosticos::agregarInformacion(const std::string& codigo, int linea,
                                            const std::string& mensaje, int columna) {
    listaDiagnosticos.push_back(Diagnostico::crearInformacion(codigo, linea, mensaje, columna));
}

bool GestorDiagnosticos::tieneErrores() const {
    for (const auto& diagnostico : listaDiagnosticos) {
        if (diagnostico.severidad == SeveridadDiagnostico::Error) {
            return true;
        }
    }
    return false;
}

bool GestorDiagnosticos::estaVacio() const {
    return listaDiagnosticos.empty();
}

size_t GestorDiagnosticos::cantidadTotal() const {
    return listaDiagnosticos.size();
}

size_t GestorDiagnosticos::cantidadErrores() const {
    size_t contador = 0;
    for (const auto& diagnostico : listaDiagnosticos) {
        if (diagnostico.severidad == SeveridadDiagnostico::Error) {
            contador++;
        }
    }
    return contador;
}

size_t GestorDiagnosticos::cantidadAdvertencias() const {
    size_t contador = 0;
    for (const auto& diagnostico : listaDiagnosticos) {
        if (diagnostico.severidad == SeveridadDiagnostico::Advertencia) {
            contador++;
        }
    }
    return contador;
}

const std::vector<Diagnostico>& GestorDiagnosticos::obtenerTodos() const {
    return listaDiagnosticos;
}

void GestorDiagnosticos::limpiar() {
    listaDiagnosticos.clear();
}

void GestorDiagnosticos::imprimir(std::ostream& salida) const {
    for (const auto& diagnostico : listaDiagnosticos) {
        salida << diagnostico.formatear() << "\n";
    }
}
