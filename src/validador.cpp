#include "validador.hpp"
#include "tabla_registros.hpp"

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

    // 4. Validar instrucciones de dos operandos (mov, add, sub)
    if (instruccion.esInstruccion(CodigoOperacion::Mov) ||
        instruccion.esInstruccion(CodigoOperacion::Add) ||
        instruccion.esInstruccion(CodigoOperacion::Sub)) {

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

        // Operando 0: Destino (debe ser obligatoriamente un registro de 64 bits)
        const Operando& opDestino = instruccion.operando(0);
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
        const Operando& opFuente = instruccion.operando(1);
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
    for (const auto& instruccion : instrucciones) {
        if (!validarInstruccion(instruccion, gestor)) {
            todoValido = false;
        }
    }
    return todoValido;
}
