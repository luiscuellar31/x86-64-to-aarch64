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
        case CodigoOperacion::And:
            return "and";
        case CodigoOperacion::Or:
            return "or";
        case CodigoOperacion::Xor:
            return "xor";
        case CodigoOperacion::Cmp:
            return "cmp";
        case CodigoOperacion::Jmp:
            return "jmp";
        case CodigoOperacion::Je:
            return "je";
        case CodigoOperacion::Jne:
            return "jne";
        case CodigoOperacion::Jl:
            return "jl";
        case CodigoOperacion::Jle:
            return "jle";
        case CodigoOperacion::Jg:
            return "jg";
        case CodigoOperacion::Jge:
            return "jge";
        case CodigoOperacion::Push:
            return "push";
        case CodigoOperacion::Pop:
            return "pop";
        case CodigoOperacion::Call:
            return "call";
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
    if (texto == "and") {
        return CodigoOperacion::And;
    }
    if (texto == "or") {
        return CodigoOperacion::Or;
    }
    if (texto == "xor") {
        return CodigoOperacion::Xor;
    }
    if (texto == "cmp") {
        return CodigoOperacion::Cmp;
    }
    if (texto == "jmp") {
        return CodigoOperacion::Jmp;
    }
    if (texto == "je") {
        return CodigoOperacion::Je;
    }
    if (texto == "jne") {
        return CodigoOperacion::Jne;
    }
    if (texto == "jl") {
        return CodigoOperacion::Jl;
    }
    if (texto == "jle") {
        return CodigoOperacion::Jle;
    }
    if (texto == "jg") {
        return CodigoOperacion::Jg;
    }
    if (texto == "jge") {
        return CodigoOperacion::Jge;
    }
    if (texto == "push") {
        return CodigoOperacion::Push;
    }
    if (texto == "pop") {
        return CodigoOperacion::Pop;
    }
    if (texto == "call") {
        return CodigoOperacion::Call;
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
        case TipoOperando::Memoria:
            return "Memoria";
        case TipoOperando::Desconocido:
        default:
            return "Desconocido";
    }
}

OperandoMemoria::OperandoMemoria()
    : registroBase(""), desplazamiento(0), anchoBits(64) {
}

OperandoMemoria::OperandoMemoria(const std::string& base, int64_t desp, int ancho)
    : registroBase(base), desplazamiento(desp), anchoBits(ancho) {
}

std::string OperandoMemoria::aTexto() const {
    if (desplazamiento == 0) {
        return "[" + registroBase + "]";
    }
    if (desplazamiento > 0) {
        return "[" + registroBase + " + " + std::to_string(desplazamiento) + "]";
    }
    return "[" + registroBase + " - " + std::to_string(-desplazamiento) + "]";
}

Operando::Operando()
    : tipo(TipoOperando::Desconocido), valor(""), valorNumerico(0), memoria() {
}

Operando::Operando(TipoOperando tipoOperando, const std::string& valorTexto, int64_t valorNum)
    : tipo(tipoOperando), valor(valorTexto), valorNumerico(valorNum), memoria() {
}

Operando::Operando(const OperandoMemoria& opMemoria)
    : tipo(TipoOperando::Memoria), valor(opMemoria.aTexto()), valorNumerico(0), memoria(opMemoria) {
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

bool Operando::esMemoria() const {
    return tipo == TipoOperando::Memoria;
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

Operando Operando::crearMemoria(const std::string& registroBase, int64_t desplazamiento, int anchoBits) {
    OperandoMemoria mem(registroBase, desplazamiento, anchoBits);
    return Operando(mem);
}

Operando Operando::crearMemoria(const OperandoMemoria& opMemoria) {
    return Operando(opMemoria);
}

std::string Operando::aTexto() const {
    if (tipo == TipoOperando::Memoria) {
        return "Memoria(" + memoria.aTexto() + ")";
    }
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

bool Instruccion::esSalto() const {
    return codigoOperacion == CodigoOperacion::Jmp || esSaltoCondicional();
}

bool Instruccion::esSaltoCondicional() const {
    return codigoOperacion == CodigoOperacion::Je ||
           codigoOperacion == CodigoOperacion::Jne ||
           codigoOperacion == CodigoOperacion::Jl ||
           codigoOperacion == CodigoOperacion::Jle ||
           codigoOperacion == CodigoOperacion::Jg ||
           codigoOperacion == CodigoOperacion::Jge;
}

bool Instruccion::esStack() const {
    return codigoOperacion == CodigoOperacion::Push ||
           codigoOperacion == CodigoOperacion::Pop;
}

bool Instruccion::esLlamada() const {
    return codigoOperacion == CodigoOperacion::Call;
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
        } else if (operandoActual.esMemoria()) {
            salida << " (base: " << operandoActual.memoria.registroBase
                   << ", disp: " << operandoActual.memoria.desplazamiento
                   << ", ancho: " << operandoActual.memoria.anchoBits << " bits)";
        }
        salida << "\n";
    }
}
