#include "traductor.hpp"
#include "analizador_sintactico.hpp"
#include "validador.hpp"
#include "tabla_registros.hpp"
#include <sstream>

InstruccionAArch64 Traductor::traducirInstruccion(const Instruccion& instruccion, MapeoTraduccion& mapeoSalida) {
    mapeoSalida.lineaOrigenX86 = instruccion.numeroLinea;

    // 1. Etiqueta pura (ej. "inicio:")
    if (instruccion.esEtiquetaPura()) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Etiqueta, instruccion.numeroLinea);
        instAArch.etiqueta = instruccion.etiqueta;

        mapeoSalida.reglaId = "ETIQUETA";
        mapeoSalida.explicacionCorta = "Definicion de etiqueta simbolica.";
        mapeoSalida.codigoOrigen = instruccion.etiqueta + ":";
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // 2. Instruccion ret
    if (instruccion.esInstruccion(CodigoOperacion::Ret)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Ret, instruccion.numeroLinea);

        mapeoSalida.reglaId = "RET_SIMPLE";
        mapeoSalida.explicacionCorta = "Retorno de funcion; en AArch64 bifurca a la direccion guardada en link register (x30/lr).";
        mapeoSalida.codigoOrigen = "ret";
        mapeoSalida.codigoDestino = "ret";
        return instAArch;
    }

    // 3. Instrucciones de salto (jmp, je, jne, jl, jle, jg, jge)
    if (instruccion.esSalto()) {
        std::string etiquetaDestino = instruccion.operando(0).valor;
        CodigoOperacionAArch64 opcAArch = CodigoOperacionAArch64::B;
        std::string reglaId = "JUMP_UNCOND";
        std::string explicacion = "Salto incondicional relativo; se traduce a 'b' (branch) en AArch64.";

        switch (instruccion.codigoOperacion) {
            case CodigoOperacion::Jmp:
                opcAArch = CodigoOperacionAArch64::B;
                reglaId = "JUMP_UNCOND";
                explicacion = "Salto incondicional relativo; se traduce a 'b' (branch) en AArch64.";
                break;
            case CodigoOperacion::Je:
                opcAArch = CodigoOperacionAArch64::BEq;
                reglaId = "JUMP_COND_EQ";
                explicacion = "Salto condicional por igualdad / cero (ZF=1); se traduce a 'b.eq' en AArch64.";
                break;
            case CodigoOperacion::Jne:
                opcAArch = CodigoOperacionAArch64::BNe;
                reglaId = "JUMP_COND_NE";
                explicacion = "Salto condicional por no igualdad / no cero (ZF=0); se traduce a 'b.ne' en AArch64.";
                break;
            case CodigoOperacion::Jl:
                opcAArch = CodigoOperacionAArch64::BLt;
                reglaId = "JUMP_COND_LT";
                explicacion = "Salto condicional si menor con signo (SF != OF); se traduce a 'b.lt' en AArch64.";
                break;
            case CodigoOperacion::Jle:
                opcAArch = CodigoOperacionAArch64::BLe;
                reglaId = "JUMP_COND_LE";
                explicacion = "Salto condicional si menor o igual con signo (ZF=1 o SF != OF); se traduce a 'b.le' en AArch64.";
                break;
            case CodigoOperacion::Jg:
                opcAArch = CodigoOperacionAArch64::BGt;
                reglaId = "JUMP_COND_GT";
                explicacion = "Salto condicional si mayor con signo (ZF=0 y SF == OF); se traduce a 'b.gt' en AArch64.";
                break;
            case CodigoOperacion::Jge:
                opcAArch = CodigoOperacionAArch64::BGe;
                reglaId = "JUMP_COND_GE";
                explicacion = "Salto condicional si mayor o igual con signo (SF == OF); se traduce a 'b.ge' en AArch64.";
                break;
            default:
                break;
        }

        InstruccionAArch64 instAArch(opcAArch, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearEtiqueta(etiquetaDestino));

        mapeoSalida.reglaId = reglaId;
        mapeoSalida.explicacionCorta = explicacion;
        mapeoSalida.codigoOrigen = codigoOperacionATexto(instruccion.codigoOperacion) + " " + etiquetaDestino;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // 4. Instrucciones de dos operandos (mov, add, sub, and, or, xor, cmp)
    std::string regDestinoX86 = instruccion.operando(0).valor;
    std::string regDestinoAArch = obtenerEquivalenteAArch64(regDestinoX86);
    const Operando& opFuente = instruccion.operando(1);

    // Caso A: mov
    if (instruccion.esInstruccion(CodigoOperacion::Mov)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Mov, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "MOV_REG_REG";
            mapeoSalida.explicacionCorta = "Copia directa de 64 bits entre registros.";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "MOV_REG_IMM";
            mapeoSalida.explicacionCorta = "Carga de constante inmediata en registro de 64 bits.";
        }

        mapeoSalida.codigoOrigen = "mov " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso B: add
    if (instruccion.esInstruccion(CodigoOperacion::Add)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Add, instruccion.numeroLinea);
        // En AArch64: add destino, fuente1, fuente2
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "ARITH_ADD_REG";
            mapeoSalida.explicacionCorta = "x86-64 usa destino destructivo de 2 operandos; AArch64 requiere tres operandos explicitos (destino, fuente1, fuente2).";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "ARITH_ADD_IMM";
            mapeoSalida.explicacionCorta = "Suma inmediata con tres operandos explicitos en AArch64.";
        }

        mapeoSalida.codigoOrigen = "add " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso C: sub
    if (instruccion.esInstruccion(CodigoOperacion::Sub)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Sub, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "ARITH_SUB_REG";
            mapeoSalida.explicacionCorta = "Resta de registros con tres operandos explicitos en AArch64 (destino = fuente1 - fuente2).";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "ARITH_SUB_IMM";
            mapeoSalida.explicacionCorta = "Resta de constante inmediata con tres operandos explicitos en AArch64.";
        }

        mapeoSalida.codigoOrigen = "sub " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso D: and
    if (instruccion.esInstruccion(CodigoOperacion::And)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::And, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "LOGIC_AND_REG";
            mapeoSalida.explicacionCorta = "Operacion logica AND bit a bit con tres operandos explicitos en AArch64.";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "LOGIC_AND_IMM";
            mapeoSalida.explicacionCorta = "Operacion logica AND bit a bit con constante inmediata en AArch64.";
        }

        mapeoSalida.codigoOrigen = "and " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso E: or
    if (instruccion.esInstruccion(CodigoOperacion::Or)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Orr, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "LOGIC_OR_REG";
            mapeoSalida.explicacionCorta = "Operacion logica OR inclusiva bit a bit; en AArch64 el mnemonico es 'orr' con tres operandos explicitos.";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "LOGIC_OR_IMM";
            mapeoSalida.explicacionCorta = "Operacion logica OR inclusiva con constante inmediata; en AArch64 el mnemonico es 'orr'.";
        }

        mapeoSalida.codigoOrigen = "or " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso F: xor
    if (instruccion.esInstruccion(CodigoOperacion::Xor)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Eor, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "LOGIC_XOR_REG";
            mapeoSalida.explicacionCorta = "Operacion logica XOR (OR exclusiva); en AArch64 el mnemonico es 'eor' con tres operandos explicitos.";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "LOGIC_XOR_IMM";
            mapeoSalida.explicacionCorta = "Operacion logica XOR con constante inmediata; en AArch64 el mnemonico es 'eor'.";
        }

        mapeoSalida.codigoOrigen = "xor " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Caso G: cmp
    if (instruccion.esInstruccion(CodigoOperacion::Cmp)) {
        InstruccionAArch64 instAArch(CodigoOperacionAArch64::Cmp, instruccion.numeroLinea);
        instAArch.agregarOperando(OperandoAArch64::crearRegistro(regDestinoAArch));

        if (opFuente.esRegistro()) {
            std::string regFuenteAArch = obtenerEquivalenteAArch64(opFuente.valor);
            instAArch.agregarOperando(OperandoAArch64::crearRegistro(regFuenteAArch));

            mapeoSalida.reglaId = "COND_CMP_REG";
            mapeoSalida.explicacionCorta = "Comparacion entre registros de 64 bits; en AArch64 es un alias de 'subs xzr, xD, xS' que actualiza flags NZCV.";
        } else {
            instAArch.agregarOperando(OperandoAArch64::crearInmediato(opFuente.valor, opFuente.valorNumerico));

            mapeoSalida.reglaId = "COND_CMP_IMM";
            mapeoSalida.explicacionCorta = "Comparacion de registro con constante inmediata; en AArch64 actualiza flags NZCV descartando el resultado.";
        }

        mapeoSalida.codigoOrigen = "cmp " + regDestinoX86 + ", " + opFuente.valor;
        mapeoSalida.codigoDestino = instAArch.emitirTexto();
        return instAArch;
    }

    // Por defecto
    InstruccionAArch64 desconocido(CodigoOperacionAArch64::Desconocido, instruccion.numeroLinea);
    mapeoSalida.reglaId = "DESCONOCIDO";
    mapeoSalida.explicacionCorta = "Instruccion no reconocida.";
    return desconocido;
}

ResultadoTraduccion Traductor::traducirInstrucciones(const std::vector<Instruccion>& instrucciones) {
    ResultadoTraduccion resultado;
    GestorDiagnosticos gestor;

    // Paso 1: Validacion semantica de todo el programa
    bool esValido = Validador::validarPrograma(instrucciones, gestor);
    if (!esValido) {
        for (const auto& diag : gestor.obtenerTodos()) {
            resultado.agregarDiagnostico(diag);
        }
        resultado.exito = false;
        return resultado;
    }

    // Paso 2: Generacion y mapeo determinista hacia AArch64
    std::stringstream codigoEmitido;
    int lineaAArch64Actual = 1;

    for (const auto& inst : instrucciones) {
        MapeoTraduccion mapeo;
        InstruccionAArch64 aarchInst = traducirInstruccion(inst, mapeo);

        std::string textoLinea = aarchInst.emitirTexto();
        if (!aarchInst.esEtiquetaPura()) {
            codigoEmitido << "    " << textoLinea << "\n";
        } else {
            codigoEmitido << textoLinea << "\n";
        }

        mapeo.lineasDestinoAArch64.push_back(lineaAArch64Actual);
        resultado.agregarMapeo(mapeo);
        lineaAArch64Actual++;
    }

    resultado.codigoGenerado = codigoEmitido.str();
    resultado.exito = true;
    return resultado;
}

ResultadoTraduccion Traductor::traducir(const std::string& codigoFuente) {
    std::vector<Instruccion> instrucciones = AnalizadorSintactico::analizarPrograma(codigoFuente);
    return traducirInstrucciones(instrucciones);
}
