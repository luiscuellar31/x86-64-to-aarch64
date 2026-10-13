#include "traductor.hpp"
#include "analizador_sintactico.hpp"
#include "validador.hpp"
#include "tabla_registros.hpp"
#include <sstream>

static InstruccionAArch64 traducirEtiqueta(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Etiqueta, instruccion.numeroLinea);
    instruccionDestino.etiqueta = instruccion.etiqueta;

    mapeoSalida.reglaId = "ETIQUETA";
    mapeoSalida.explicacionCorta = "Definicion de etiqueta simbolica.";
    mapeoSalida.codigoOrigen = instruccion.etiqueta + ":";
    return instruccionDestino;
}

static InstruccionAArch64 traducirRetorno(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Ret, instruccion.numeroLinea);

    mapeoSalida.reglaId = "RET_SIMPLE";
    mapeoSalida.explicacionCorta = "Retorno de funcion; en AArch64 bifurca a la direccion guardada en link register (x30/lr).";
    mapeoSalida.codigoOrigen = "ret";
    return instruccionDestino;
}

static InstruccionAArch64 traducirSalto(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    std::string etiquetaDestino = instruccion.operando(0).valor;
    CodigoOperacionAArch64 codigoOperacionDestino = CodigoOperacionAArch64::B;
    std::string reglaId = "JUMP_UNCOND";
    std::string explicacion = "Salto incondicional relativo; se traduce a 'b' (branch) en AArch64.";

    switch (instruccion.codigoOperacion) {
        case CodigoOperacion::Jmp:
            codigoOperacionDestino = CodigoOperacionAArch64::B;
            reglaId = "JUMP_UNCOND";
            explicacion = "Salto incondicional relativo; se traduce a 'b' (branch) en AArch64.";
            break;
        case CodigoOperacion::Je:
            codigoOperacionDestino = CodigoOperacionAArch64::BEq;
            reglaId = "JUMP_COND_EQ";
            explicacion = "Salto condicional por igualdad / cero (ZF=1); se traduce a 'b.eq' en AArch64.";
            break;
        case CodigoOperacion::Jne:
            codigoOperacionDestino = CodigoOperacionAArch64::BNe;
            reglaId = "JUMP_COND_NE";
            explicacion = "Salto condicional por no igualdad / no cero (ZF=0); se traduce a 'b.ne' en AArch64.";
            break;
        case CodigoOperacion::Jl:
            codigoOperacionDestino = CodigoOperacionAArch64::BLt;
            reglaId = "JUMP_COND_LT";
            explicacion = "Salto condicional si menor con signo (SF != OF); se traduce a 'b.lt' en AArch64.";
            break;
        case CodigoOperacion::Jle:
            codigoOperacionDestino = CodigoOperacionAArch64::BLe;
            reglaId = "JUMP_COND_LE";
            explicacion = "Salto condicional si menor o igual con signo (ZF=1 o SF != OF); se traduce a 'b.le' en AArch64.";
            break;
        case CodigoOperacion::Jg:
            codigoOperacionDestino = CodigoOperacionAArch64::BGt;
            reglaId = "JUMP_COND_GT";
            explicacion = "Salto condicional si mayor con signo (ZF=0 y SF == OF); se traduce a 'b.gt' en AArch64.";
            break;
        case CodigoOperacion::Jge:
            codigoOperacionDestino = CodigoOperacionAArch64::BGe;
            reglaId = "JUMP_COND_GE";
            explicacion = "Salto condicional si mayor o igual con signo (SF == OF); se traduce a 'b.ge' en AArch64.";
            break;
        default:
            break;
    }

    InstruccionAArch64 instruccionDestino(codigoOperacionDestino, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearEtiqueta(etiquetaDestino));

    mapeoSalida.reglaId = reglaId;
    mapeoSalida.explicacionCorta = explicacion;
    mapeoSalida.codigoOrigen = codigoOperacionATexto(instruccion.codigoOperacion) + " " + etiquetaDestino;
    return instruccionDestino;
}

static InstruccionAArch64 traducirPush(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Str, instruccion.numeroLinea);
    std::string registroX86 = instruccion.operando(0).valor;
    std::string registroAArch64 = obtenerEquivalenteAArch64(registroX86);

    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearMemoria("sp", -16, ModoIndexadoAArch64::PreIndexado));

    mapeoSalida.reglaId = "STACK_PUSH_REG";
    mapeoSalida.explicacionCorta = "En AArch64 no existe instruccion 'push'; se traduce a 'str " + registroAArch64 + ", [sp, #-16]!' con pre-indexado para preservar la alineacion de 16 bytes de AAPCS64.";
    mapeoSalida.codigoOrigen = "push " + registroX86;
    return instruccionDestino;
}

static InstruccionAArch64 traducirPop(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Ldr, instruccion.numeroLinea);
    std::string registroX86 = instruccion.operando(0).valor;
    std::string registroAArch64 = obtenerEquivalenteAArch64(registroX86);

    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearMemoria("sp", 16, ModoIndexadoAArch64::PostIndexado));

    mapeoSalida.reglaId = "STACK_POP_REG";
    mapeoSalida.explicacionCorta = "En AArch64 no existe instruccion 'pop'; se traduce a 'ldr " + registroAArch64 + ", [sp], #16' con post-indexado restaurando el registro y el puntero de pila alineado.";
    mapeoSalida.codigoOrigen = "pop " + registroX86;
    return instruccionDestino;
}

static InstruccionAArch64 traducirMovimiento(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    if (operandoDestino.esRegistro() && operandoFuente.esMemoria()) {
        // Carga de memoria a registro: ldr xD, [xN, #offset]
        InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Ldr, instruccion.numeroLinea);
        std::string registroBaseAArch64 = obtenerEquivalenteAArch64(operandoFuente.memoria.registroBase);

        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
        instruccionDestino.agregarOperando(OperandoAArch64::crearMemoria(registroBaseAArch64, operandoFuente.memoria.desplazamiento));

        mapeoSalida.reglaId = "MEM_LOAD_STACK";
        mapeoSalida.explicacionCorta = "Carga de memoria local del stack a registro (ldr) con direccion base " + registroBaseAArch64 + ".";
        mapeoSalida.codigoOrigen = "mov " + registroDestinoX86 + ", " + operandoFuente.memoria.aTexto();
        return instruccionDestino;
    }

    if (operandoDestino.esMemoria() && operandoFuente.esRegistro()) {
        // Almacenamiento de registro a memoria: str xS, [xN, #offset]
        InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Str, instruccion.numeroLinea);
        std::string registroFuenteX86 = operandoFuente.valor;
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(registroFuenteX86);
        std::string registroBaseAArch64 = obtenerEquivalenteAArch64(operandoDestino.memoria.registroBase);

        // En AArch64 str, el primer operando es el registro fuente y el segundo es la ubicacion de memoria destino
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));
        instruccionDestino.agregarOperando(OperandoAArch64::crearMemoria(registroBaseAArch64, operandoDestino.memoria.desplazamiento));

        mapeoSalida.reglaId = "MEM_STORE_STACK";
        mapeoSalida.explicacionCorta = "Almacenamiento de registro a memoria local del stack (str) con direccion base " + registroBaseAArch64 + ".";
        mapeoSalida.codigoOrigen = "mov " + operandoDestino.memoria.aTexto() + ", " + registroFuenteX86;
        return instruccionDestino;
    }

    // Movimiento a registro (reg <- reg o reg <- imm)
    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Mov, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "MOV_REG_REG";
        mapeoSalida.explicacionCorta = "Copia directa de 64 bits entre registros.";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "MOV_REG_IMM";
        mapeoSalida.explicacionCorta = "Carga de constante inmediata en registro de 64 bits.";
    }

    mapeoSalida.codigoOrigen = "mov " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirSuma(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Add, instruccion.numeroLinea);
    // En AArch64: add destino, fuente1, fuente2
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "ARITH_ADD_REG";
        mapeoSalida.explicacionCorta = "x86-64 usa destino destructivo de 2 operandos; AArch64 requiere tres operandos explicitos (destino, fuente1, fuente2).";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "ARITH_ADD_IMM";
        mapeoSalida.explicacionCorta = "Suma inmediata con tres operandos explicitos en AArch64.";
    }

    mapeoSalida.codigoOrigen = "add " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirResta(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Sub, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "ARITH_SUB_REG";
        mapeoSalida.explicacionCorta = "Resta de registros con tres operandos explicitos en AArch64 (destino = fuente1 - fuente2).";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "ARITH_SUB_IMM";
        mapeoSalida.explicacionCorta = "Resta de constante inmediata con tres operandos explicitos en AArch64.";
    }

    mapeoSalida.codigoOrigen = "sub " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirAnd(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::And, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "LOGIC_AND_REG";
        mapeoSalida.explicacionCorta = "Operacion logica AND bit a bit con tres operandos explicitos en AArch64.";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "LOGIC_AND_IMM";
        mapeoSalida.explicacionCorta = "Operacion logica AND bit a bit con constante inmediata en AArch64.";
    }

    mapeoSalida.codigoOrigen = "and " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirOr(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Orr, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "LOGIC_OR_REG";
        mapeoSalida.explicacionCorta = "Operacion logica OR inclusiva bit a bit; en AArch64 el mnemonico es 'orr' con tres operandos explicitos.";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "LOGIC_OR_IMM";
        mapeoSalida.explicacionCorta = "Operacion logica OR inclusiva con constante inmediata; en AArch64 el mnemonico es 'orr'.";
    }

    mapeoSalida.codigoOrigen = "or " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirXor(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Eor, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "LOGIC_XOR_REG";
        mapeoSalida.explicacionCorta = "Operacion logica XOR (OR exclusiva); en AArch64 el mnemonico es 'eor' con tres operandos explicitos.";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "LOGIC_XOR_IMM";
        mapeoSalida.explicacionCorta = "Operacion logica XOR con constante inmediata; en AArch64 el mnemonico es 'eor'.";
    }

    mapeoSalida.codigoOrigen = "xor " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static InstruccionAArch64 traducirComparacion(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    const Operando& operandoDestino = instruccion.operando(0);
    const Operando& operandoFuente = instruccion.operando(1);
    std::string registroDestinoX86 = operandoDestino.valor;
    std::string registroDestinoAArch64;
    if (operandoDestino.esRegistro()) {
        registroDestinoAArch64 = obtenerEquivalenteAArch64(registroDestinoX86);
    }

    InstruccionAArch64 instruccionDestino(CodigoOperacionAArch64::Cmp, instruccion.numeroLinea);
    instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroDestinoAArch64));

    if (operandoFuente.esRegistro()) {
        std::string registroFuenteAArch64 = obtenerEquivalenteAArch64(operandoFuente.valor);
        instruccionDestino.agregarOperando(OperandoAArch64::crearRegistro(registroFuenteAArch64));

        mapeoSalida.reglaId = "COND_CMP_REG";
        mapeoSalida.explicacionCorta = "Comparacion entre registros de 64 bits; en AArch64 es un alias de 'subs xzr, xD, xS' que actualiza flags NZCV.";
    } else {
        instruccionDestino.agregarOperando(OperandoAArch64::crearInmediato(operandoFuente.valor, operandoFuente.valorNumerico));

        mapeoSalida.reglaId = "COND_CMP_IMM";
        mapeoSalida.explicacionCorta = "Comparacion de registro con constante inmediata; en AArch64 actualiza flags NZCV descartando el resultado.";
    }

    mapeoSalida.codigoOrigen = "cmp " + registroDestinoX86 + ", " + operandoFuente.valor;
    return instruccionDestino;
}

static std::vector<InstruccionAArch64> traducirLlamada(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    InstruccionAArch64 guardarEnlace(CodigoOperacionAArch64::Str, instruccion.numeroLinea);
    guardarEnlace.agregarOperando(OperandoAArch64::crearRegistro("x30"));
    guardarEnlace.agregarOperando(OperandoAArch64::crearMemoria("sp", -16, ModoIndexadoAArch64::PreIndexado));

    InstruccionAArch64 llamada(CodigoOperacionAArch64::Bl, instruccion.numeroLinea);
    llamada.agregarOperando(OperandoAArch64::crearEtiqueta(instruccion.operando(0).valor));

    InstruccionAArch64 restaurarEnlace(CodigoOperacionAArch64::Ldr, instruccion.numeroLinea);
    restaurarEnlace.agregarOperando(OperandoAArch64::crearRegistro("x30"));
    restaurarEnlace.agregarOperando(OperandoAArch64::crearMemoria("sp", 16, ModoIndexadoAArch64::PostIndexado));

    mapeoSalida.reglaId = "CALL_DIRECT";
    mapeoSalida.explicacionCorta = "Guarda x30 en un bloque de 16 bytes antes de bl y lo restaura al volver; conserva el retorno del llamante y la alineacion previa de sp.";
    mapeoSalida.codigoOrigen = "call " + instruccion.operando(0).valor;
    return {guardarEnlace, llamada, restaurarEnlace};
}

static InstruccionAArch64 traducirInstruccionBase(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    switch (instruccion.codigoOperacion) {
        case CodigoOperacion::Etiqueta:
            return traducirEtiqueta(instruccion, mapeoSalida);
        case CodigoOperacion::Ret:
            return traducirRetorno(instruccion, mapeoSalida);
        case CodigoOperacion::Jmp:
        case CodigoOperacion::Je:
        case CodigoOperacion::Jne:
        case CodigoOperacion::Jl:
        case CodigoOperacion::Jle:
        case CodigoOperacion::Jg:
        case CodigoOperacion::Jge:
            return traducirSalto(instruccion, mapeoSalida);
        case CodigoOperacion::Push:
            return traducirPush(instruccion, mapeoSalida);
        case CodigoOperacion::Pop:
            return traducirPop(instruccion, mapeoSalida);
        case CodigoOperacion::Mov:
            return traducirMovimiento(instruccion, mapeoSalida);
        case CodigoOperacion::Add:
            return traducirSuma(instruccion, mapeoSalida);
        case CodigoOperacion::Sub:
            return traducirResta(instruccion, mapeoSalida);
        case CodigoOperacion::And:
            return traducirAnd(instruccion, mapeoSalida);
        case CodigoOperacion::Or:
            return traducirOr(instruccion, mapeoSalida);
        case CodigoOperacion::Xor:
            return traducirXor(instruccion, mapeoSalida);
        case CodigoOperacion::Cmp:
            return traducirComparacion(instruccion, mapeoSalida);
        default:
            mapeoSalida.reglaId = "DESCONOCIDO";
            mapeoSalida.explicacionCorta = "Instruccion no reconocida.";
            return InstruccionAArch64(CodigoOperacionAArch64::Desconocido, instruccion.numeroLinea);
    }
}

std::vector<InstruccionAArch64> Traductor::traducirInstruccion(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    mapeoSalida.lineaOrigenX86 = instruccion.numeroLinea;
    std::vector<InstruccionAArch64> secuencia;
    if (instruccion.esLlamada()) {
        secuencia = traducirLlamada(instruccion, mapeoSalida);
    } else {
        secuencia.push_back(traducirInstruccionBase(instruccion, mapeoSalida));
    }

    // Una etiqueta en call debe apuntar al guardado del enlace, antes de bl.
    if (instruccion.tieneEtiqueta() && !instruccion.esEtiquetaPura()) {
        secuencia.front().etiqueta = instruccion.etiqueta;
        mapeoSalida.codigoOrigen = instruccion.etiqueta + ": " + mapeoSalida.codigoOrigen;
    }

    mapeoSalida.codigoDestino.clear();
    for (auto& instruccionDestino : secuencia) {
        if (!mapeoSalida.codigoDestino.empty()) {
            mapeoSalida.codigoDestino += "\n";
        }
        mapeoSalida.codigoDestino += instruccionDestino.emitirTexto();
        instruccionDestino.reglaId = mapeoSalida.reglaId;
        instruccionDestino.explicacion = mapeoSalida.explicacionCorta;
    }
    return secuencia;
}

ResultadoTraduccion Traductor::traducirInstrucciones(const std::vector<Instruccion>& instrucciones) {
    ResultadoTraduccion resultado;
    resultado.irOrigen = instrucciones;
    GestorDiagnosticos gestor;

    // Paso 1: Validacion semantica de todo el programa
    bool esValido = Validador::validarPrograma(instrucciones, gestor);
    if (!esValido) {
        for (const auto& diagnostico : gestor.obtenerTodos()) {
            resultado.agregarDiagnostico(diagnostico);
        }
        resultado.exito = false;
        return resultado;
    }

    // Paso 2: Generacion y mapeo determinista hacia AArch64
    std::stringstream codigoEmitido;
    int lineaAArch64Actual = 1;

    for (const auto& instruccionOrigen : instrucciones) {
        MapeoTraduccion mapeo;
        std::vector<InstruccionAArch64> secuencia = traducirInstruccion(instruccionOrigen, mapeo);

        for (const auto& instruccionDestino : secuencia) {
            std::string textoLinea = instruccionDestino.emitirTexto();
            if (!instruccionDestino.esEtiquetaPura()) {
                codigoEmitido << "    " << textoLinea << "\n";
            } else {
                codigoEmitido << textoLinea << "\n";
            }

            mapeo.lineasDestinoAArch64.push_back(lineaAArch64Actual);
            resultado.irDestino.push_back(instruccionDestino);
            lineaAArch64Actual++;
        }
        resultado.agregarMapeo(mapeo);
    }

    resultado.codigoGenerado = codigoEmitido.str();
    resultado.exito = true;
    return resultado;
}

ResultadoTraduccion Traductor::traducir(const std::string& codigoFuente) {
    std::vector<Instruccion> instrucciones = AnalizadorSintactico::analizarPrograma(codigoFuente);
    return traducirInstrucciones(instrucciones);
}
