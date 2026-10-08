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
    : tipo(TipoOperando::Desconocido), valor("") {
}

Operando::Operando(TipoOperando tipoOperando, const std::string& valorTexto)
    : tipo(tipoOperando), valor(valorTexto) {
}

Operando Operando::crearRegistro(const std::string& nombreRegistro) {
    return Operando(TipoOperando::Registro, nombreRegistro);
}

Operando Operando::crearInmediato(const std::string& valorInmediato) {
    return Operando(TipoOperando::Inmediato, valorInmediato);
}

Operando Operando::crearEtiqueta(const std::string& nombreEtiqueta) {
    return Operando(TipoOperando::Etiqueta, nombreEtiqueta);
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
               << " -> " << operandoActual.valor << "\n";
    }
}
