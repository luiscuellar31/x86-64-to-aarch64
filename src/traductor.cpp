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

    // 3. Instrucciones de dos operandos (mov, add, sub)
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
