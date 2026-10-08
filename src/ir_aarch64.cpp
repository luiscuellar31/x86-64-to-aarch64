#include "ir_aarch64.hpp"

std::string codigoOperacionAArch64ATexto(CodigoOperacionAArch64 codigo) {
    switch (codigo) {
        case CodigoOperacionAArch64::Etiqueta:
            return "etiqueta";
        case CodigoOperacionAArch64::Mov:
            return "mov";
        case CodigoOperacionAArch64::Add:
            return "add";
        case CodigoOperacionAArch64::Sub:
            return "sub";
        case CodigoOperacionAArch64::And:
            return "and";
        case CodigoOperacionAArch64::Orr:
            return "orr";
        case CodigoOperacionAArch64::Eor:
            return "eor";
        case CodigoOperacionAArch64::Cmp:
            return "cmp";
        case CodigoOperacionAArch64::B:
            return "b";
        case CodigoOperacionAArch64::BEq:
            return "b.eq";
        case CodigoOperacionAArch64::BNe:
            return "b.ne";
        case CodigoOperacionAArch64::BLt:
            return "b.lt";
        case CodigoOperacionAArch64::BLe:
            return "b.le";
        case CodigoOperacionAArch64::BGt:
            return "b.gt";
        case CodigoOperacionAArch64::BGe:
            return "b.ge";
        case CodigoOperacionAArch64::Ret:
            return "ret";
        case CodigoOperacionAArch64::Desconocido:
        default:
            return "desconocido";
    }
}

OperandoAArch64::OperandoAArch64()
    : tipo(TipoOperandoAArch64::Desconocido), valor(""), valorNumerico(0) {
}

OperandoAArch64::OperandoAArch64(TipoOperandoAArch64 tipoOp, const std::string& valorTexto, int64_t valorNum)
    : tipo(tipoOp), valor(valorTexto), valorNumerico(valorNum) {
}

bool OperandoAArch64::esRegistro() const {
    return tipo == TipoOperandoAArch64::Registro;
}

bool OperandoAArch64::esInmediato() const {
    return tipo == TipoOperandoAArch64::Inmediato;
}

bool OperandoAArch64::esEtiqueta() const {
    return tipo == TipoOperandoAArch64::Etiqueta;
}

OperandoAArch64 OperandoAArch64::crearRegistro(const std::string& nombreRegistro) {
    return OperandoAArch64(TipoOperandoAArch64::Registro, nombreRegistro, 0);
}

OperandoAArch64 OperandoAArch64::crearInmediato(int64_t valorNum) {
    return OperandoAArch64(TipoOperandoAArch64::Inmediato, std::to_string(valorNum), valorNum);
}

OperandoAArch64 OperandoAArch64::crearInmediato(const std::string& valorTexto, int64_t valorNum) {
    return OperandoAArch64(TipoOperandoAArch64::Inmediato, valorTexto, valorNum);
}

OperandoAArch64 OperandoAArch64::crearEtiqueta(const std::string& nombreEtiqueta) {
    return OperandoAArch64(TipoOperandoAArch64::Etiqueta, nombreEtiqueta, 0);
}

std::string OperandoAArch64::emitirTexto() const {
    if (tipo == TipoOperandoAArch64::Inmediato) {
        if (!valor.empty() && valor[0] == '#') {
            return valor;
        }
        return "#" + valor;
    }
    return valor;
}

InstruccionAArch64::InstruccionAArch64(CodigoOperacionAArch64 codigo, int linea)
    : codigoOperacion(codigo), etiqueta(""), lineaOrigen(linea), reglaId(""), explicacion("") {
}

bool InstruccionAArch64::esEtiquetaPura() const {
    return codigoOperacion == CodigoOperacionAArch64::Etiqueta;
}

void InstruccionAArch64::agregarOperando(const OperandoAArch64& operando) {
    operandos.push_back(operando);
}

std::string InstruccionAArch64::emitirTexto() const {
    if (esEtiquetaPura()) {
        return etiqueta + ":";
    }

    std::string cuerpo = codigoOperacionAArch64ATexto(codigoOperacion);

    if (!operandos.empty()) {
        cuerpo += " ";
        for (size_t indice = 0; indice < operandos.size(); ++indice) {
            if (indice > 0) {
                cuerpo += ", ";
            }
            cuerpo += operandos[indice].emitirTexto();
        }
    }

    if (!etiqueta.empty()) {
        return etiqueta + ": " + cuerpo;
    }

    return cuerpo;
}
