#include "instruccion.hpp"
#include <cctype>

std::string codigoOperacionATexto(CodigoOperacion codigo) {
    switch (codigo) {
        case CodigoOperacion::Etiqueta:
            return "etiqueta";
        case CodigoOperacion::Mov:
            return "mov";
        case CodigoOperacion::Add:
            return "add";
        case CodigoOperacion::Sub:
            return "sub";
        case CodigoOperacion::Ret:
            return "ret";
        case CodigoOperacion::Desconocido:
        default:
            return "desconocido";
    }
}

CodigoOperacion textoACodigoOperacion(const std::string& texto) {
    if (texto == "mov") {
        return CodigoOperacion::Mov;
    }
    if (texto == "add") {
        return CodigoOperacion::Add;
    }
    if (texto == "sub") {
        return CodigoOperacion::Sub;
    }
    if (texto == "ret") {
        return CodigoOperacion::Ret;
    }
    return CodigoOperacion::Desconocido;
}

std::string tipoOperandoATexto(TipoOperando tipo) {
    switch (tipo) {
        case TipoOperando::Registro:
            return "Registro";
        case TipoOperando::Inmediato:
            return "Inmediato";
        case TipoOperando::Etiqueta:
            return "Etiqueta";
        case TipoOperando::Desconocido:
        default:
            return "Desconocido";
    }
}

Operando::Operando()
    : tipo(TipoOperando::Desconocido), valor(""), valorNumerico(0) {
}

Operando::Operando(TipoOperando tipoOperando, const std::string& valorTexto, int64_t valorNum)
    : tipo(tipoOperando), valor(valorTexto), valorNumerico(valorNum) {
}

bool Operando::esRegistro() const {
    return tipo == TipoOperando::Registro;
}

bool Operando::esInmediato() const {
    return tipo == TipoOperando::Inmediato;
}

bool Operando::esEtiqueta() const {
    return tipo == TipoOperando::Etiqueta;
}

Operando Operando::crearRegistro(const std::string& nombreRegistro) {
    return Operando(TipoOperando::Registro, nombreRegistro, 0);
}

Operando Operando::crearInmediato(const std::string& valorInmediato) {
    int64_t num = 0;
    try {
        num = std::stoll(valorInmediato, nullptr, 0);
    } catch (...) {
        num = 0;
    }
    return Operando(TipoOperando::Inmediato, valorInmediato, num);
}

Operando Operando::crearInmediato(int64_t valorNum) {
    return Operando(TipoOperando::Inmediato, std::to_string(valorNum), valorNum);
}

Operando Operando::crearEtiqueta(const std::string& nombreEtiqueta) {
    return Operando(TipoOperando::Etiqueta, nombreEtiqueta, 0);
}

std::string Operando::aTexto() const {
    return tipoOperandoATexto(tipo) + "(\"" + valor + "\")";
}

Instruccion::Instruccion(CodigoOperacion codigo, int linea)
    : codigoOperacion(codigo), numeroLinea(linea), etiqueta("") {
}

Instruccion::Instruccion(CodigoOperacion codigo, const std::vector<Operando>& listaOperandos, int linea)
    : codigoOperacion(codigo), operandos(listaOperandos), numeroLinea(linea), etiqueta("") {
}

bool Instruccion::tieneEtiqueta() const {
    return !etiqueta.empty();
}

bool Instruccion::esEtiquetaPura() const {
    return codigoOperacion == CodigoOperacion::Etiqueta;
}

bool Instruccion::esInstruccion(CodigoOperacion codigo) const {
    return codigoOperacion == codigo;
}

size_t Instruccion::cantidadOperandos() const {
    return operandos.size();
}

const Operando& Instruccion::operando(size_t indice) const {
    return operandos.at(indice);
}

void Instruccion::agregarOperando(const Operando& operando) {
    operandos.push_back(operando);
}

void Instruccion::imprimir(std::ostream& salida) const {
    salida << "Instruccion (linea " << numeroLinea << "):\n";

    if (tieneEtiqueta()) {
        salida << "  Etiqueta: " << etiqueta << "\n";
    }

    salida << "  Opcode: " << codigoOperacionATexto(codigoOperacion) << "\n";
    salida << "  Operandos:\n";

    if (operandos.empty()) {
        salida << "    (ninguno)\n";
        return;
    }

    for (size_t indice = 0; indice < operandos.size(); ++indice) {
        const Operando& operandoActual = operandos[indice];
        salida << "    [" << indice << "] "
               << tipoOperandoATexto(operandoActual.tipo)
               << " -> " << operandoActual.valor;
        if (operandoActual.esInmediato()) {
            salida << " (valor numerico: " << operandoActual.valorNumerico << ")";
        }
        salida << "\n";
    }
}
