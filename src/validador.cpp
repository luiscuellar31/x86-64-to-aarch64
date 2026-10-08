#include "validador.hpp"
#include "tabla_registros.hpp"
#include <unordered_set>

bool Validador::validarInstruccion(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    int linea = instruccion.numeroLinea;

    // 1. Validar definiciones de etiqueta pura (ej. "inicio:")
    if (instruccion.esEtiquetaPura()) {
        if (instruccion.cantidadOperandos() > 0) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "las definiciones de etiqueta no admiten operandos");
            return false;
        }
        return true;
    }

    // 2. Validar mnemónico soportado en V0
    if (instruccion.codigoOperacion == CodigoOperacion::Desconocido) {
        gestor.agregarError(CodigosDiagnostico::kInstruccionDesconocida, linea,
                            "instruccion no soportada o desconocida");
        return false;
    }

    // 3. Validar instruccion ret
    if (instruccion.esInstruccion(CodigoOperacion::Ret)) {
        if (instruccion.cantidadOperandos() > 0) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion 'ret' no admite operandos en este subconjunto");
            return false;
        }
        return true;
    }

    // 4. Validar instrucciones de salto (jmp, je, jne, jl, jle, jg, jge)
    if (instruccion.esSalto()) {
        std::string nombreMnemonic = codigoOperacionATexto(instruccion.codigoOperacion);

        if (instruccion.cantidadOperandos() < 1) {
            gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' requiere 1 operando (etiqueta destino)");
            return false;
        }

        if (instruccion.cantidadOperandos() > 1) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' solo admite 1 operando (etiqueta destino)");
            return false;
        }

        const Operando& opDestino = instruccion.operando(0);
        if (!opDestino.esEtiqueta()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el operando de '" + nombreMnemonic + "' debe ser una etiqueta valida");
            return false;
        }

        return true;
    }

    // 5. Validar instruccion de llamada (call)
    if (instruccion.esLlamada()) {
        if (instruccion.cantidadOperandos() < 1) {
            gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, linea,
                                "la instruccion 'call' requiere 1 operando (etiqueta destino)");
            return false;
        }

        if (instruccion.cantidadOperandos() > 1) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion 'call' solo admite 1 operando (etiqueta destino)");
            return false;
        }

        const Operando& opDestino = instruccion.operando(0);
        if (!opDestino.esEtiqueta()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el operando de 'call' debe ser una etiqueta valida");
            return false;
        }

        return true;
    }

    // 6. Validar instrucciones de stack (push, pop)
    if (instruccion.esStack()) {
        std::string nombreMnemonic = codigoOperacionATexto(instruccion.codigoOperacion);

        if (instruccion.cantidadOperandos() < 1) {
            gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' requiere 1 operando (registro de 64 bits)");
            return false;
        }

        if (instruccion.cantidadOperandos() > 1) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' solo admite 1 operando en este subconjunto");
            return false;
        }

        const Operando& op = instruccion.operando(0);

        if (op.esInmediato()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "la instruccion '" + nombreMnemonic + "' no admite constantes inmediatas en este subconjunto");
            return false;
        }

        if (op.esMemoria()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "la instruccion '" + nombreMnemonic + "' no admite operandos de memoria en este subconjunto");
            return false;
        }

        if (op.esRegistro()) {
            if (esSubregistroX86(op.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + op.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(op.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro '" + op.valor + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else {
            if (esSubregistroX86(op.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + op.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el operando de '" + nombreMnemonic + "' debe ser un registro de 64 bits");
            return false;
        }

        return true;
    }

    // 6. Validar instruccion mov (soporta reg<-reg, reg<-imm, reg<-mem, mem<-reg)
    if (instruccion.esInstruccion(CodigoOperacion::Mov)) {
        if (instruccion.cantidadOperandos() < 2) {
            gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, linea,
                                "la instruccion 'mov' requiere 2 operandos (destino, fuente)");
            return false;
        }

        if (instruccion.cantidadOperandos() > 2) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion 'mov' admite un maximo de 2 operandos en x86-64");
            return false;
        }

        const Operando& opDestino = instruccion.operando(0);
        const Operando& opFuente = instruccion.operando(1);

        // Operando 0: Destino (registro o memoria)
        if (opDestino.esInmediato()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el destino de 'mov' no puede ser una constante inmediata");
            return false;
        }

        if (opDestino.esRegistro()) {
            if (esSubregistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opDestino.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro destino '" + opDestino.valor + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else if (opDestino.esMemoria()) {
            if (esSubregistroX86(opDestino.memoria.registroBase)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro base '" + opDestino.memoria.registroBase + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opDestino.memoria.registroBase)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro base '" + opDestino.memoria.registroBase + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else {
            if (esSubregistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opDestino.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el primer operando de 'mov' debe ser un registro de destino o una referencia a memoria");
            return false;
        }

        // Operando 1: Fuente (registro, inmediato o memoria)
        if (opFuente.esRegistro()) {
            if (esSubregistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opFuente.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro fuente '" + opFuente.valor + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else if (opFuente.esInmediato()) {
            if (opDestino.esMemoria()) {
                gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                    "no se admite almacenar una constante inmediata directamente en memoria en este subconjunto");
                return false;
            }
        } else if (opFuente.esMemoria()) {
            if (opDestino.esMemoria()) {
                gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                    "no se admite transferencia directa de memoria a memoria en mov");
                return false;
            }
            if (esSubregistroX86(opFuente.memoria.registroBase)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro base '" + opFuente.memoria.registroBase + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opFuente.memoria.registroBase)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro base '" + opFuente.memoria.registroBase + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else {
            if (esSubregistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opFuente.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el operando fuente de 'mov' debe ser un registro, un valor inmediato o una referencia a memoria");
            return false;
        }

        return true;
    }

    // 6. Validar instrucciones de dos operandos aritmeticas y logicas (add, sub, and, or, xor, cmp)
    if (instruccion.esInstruccion(CodigoOperacion::Add) ||
        instruccion.esInstruccion(CodigoOperacion::Sub) ||
        instruccion.esInstruccion(CodigoOperacion::And) ||
        instruccion.esInstruccion(CodigoOperacion::Or) ||
        instruccion.esInstruccion(CodigoOperacion::Xor) ||
        instruccion.esInstruccion(CodigoOperacion::Cmp)) {

        std::string nombreMnemonic = codigoOperacionATexto(instruccion.codigoOperacion);

        // Validar cantidad de operandos
        if (instruccion.cantidadOperandos() < 2) {
            gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' requiere 2 operandos (destino, fuente)");
            return false;
        }

        if (instruccion.cantidadOperandos() > 2) {
            gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, linea,
                                "la instruccion '" + nombreMnemonic + "' admite un maximo de 2 operandos en x86-64");
            return false;
        }

        const Operando& opDestino = instruccion.operando(0);
        const Operando& opFuente = instruccion.operando(1);

        // En este subconjunto, memoria no esta admitida en operaciones aritmeticas ni logicas
        if (opDestino.esMemoria() || opFuente.esMemoria()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "la instruccion '" + nombreMnemonic + "' no admite operandos de memoria en este subconjunto");
            return false;
        }

        // Operando 0: Destino (debe ser obligatoriamente un registro de 64 bits)
        if (opDestino.esInmediato()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el destino de '" + nombreMnemonic + "' no puede ser una constante inmediata");
            return false;
        }

        if (opDestino.esRegistro()) {
            if (esSubregistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opDestino.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro destino '" + opDestino.valor + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else {
            if (esSubregistroX86(opDestino.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opDestino.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el primer operando de '" + nombreMnemonic + "' debe ser un registro de destino");
            return false;
        }

        // Operando 1: Fuente (puede ser registro de 64 bits o inmediato numérico)
        if (opFuente.esRegistro()) {
            if (esSubregistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opFuente.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            if (!esRegistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "el registro fuente '" + opFuente.valor + "' no pertenece al subconjunto soportado de 64 bits");
                return false;
            }
        } else if (opFuente.esInmediato()) {
            // Inmediato numérico válido
        } else {
            if (esSubregistroX86(opFuente.valor)) {
                gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, linea,
                                    "registro '" + opFuente.valor + "' no soportado en modo solo 64 bits");
                return false;
            }
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, linea,
                                "el operando fuente de '" + nombreMnemonic + "' debe ser un registro o un valor inmediato");
            return false;
        }

        return true;
    }

    // Cualquier otro caso no contemplado
    gestor.agregarError(CodigosDiagnostico::kInstruccionDesconocida, linea,
                        "forma de instruccion no soportada");
    return false;
}

bool Validador::validarPrograma(const std::vector<Instruccion>& instrucciones, GestorDiagnosticos& gestor) {
    bool todoValido = true;

    // 1. Recolectar todas las etiquetas definidas en el programa
    std::unordered_set<std::string> etiquetasDefinidas;
    for (const auto& instruccion : instrucciones) {
        if (instruccion.tieneEtiqueta()) {
            etiquetasDefinidas.insert(instruccion.etiqueta);
        }
    }

    // 2. Validar cada instrucción y la existencia de los destinos de salto
    for (const auto& instruccion : instrucciones) {
        if (!validarInstruccion(instruccion, gestor)) {
            todoValido = false;
        } else if ((instruccion.esSalto() || instruccion.esLlamada()) && instruccion.cantidadOperandos() == 1) {
            const Operando& opDestino = instruccion.operando(0);
            if (opDestino.esEtiqueta() && etiquetasDefinidas.find(opDestino.valor) == etiquetasDefinidas.end()) {
                std::string tipoDestino = instruccion.esLlamada() ? "llamada" : "salto";
                gestor.agregarError(CodigosDiagnostico::kEtiquetaNoDefinida, instruccion.numeroLinea,
                                    "etiqueta de " + tipoDestino + " '" + opDestino.valor + "' no definida en el programa");
                todoValido = false;
            }
        }
    }

    return todoValido;
}
