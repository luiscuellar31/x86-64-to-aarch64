#include "validador.hpp"
#include "tabla_registros.hpp"
#include <unordered_set>

static bool validarRegistro64(const std::string& nombreRegistro, const std::string& descripcionRegistro,
                              int numeroLinea, GestorDiagnosticos& gestor) {
    if (esSubregistroX86(nombreRegistro)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro '" + nombreRegistro + "' no soportado en modo solo 64 bits");
        return false;
    }
    if (!esRegistroX86(nombreRegistro)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            descripcionRegistro + " '" + nombreRegistro + "' no pertenece al subconjunto soportado de 64 bits");
        return false;
    }
    return true;
}

static bool validarBaseMemoria(const OperandoMemoria& memoria, int numeroLinea, GestorDiagnosticos& gestor) {
    if (esSubregistroX86(memoria.registroBase)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro base '" + memoria.registroBase + "' no soportado en modo solo 64 bits");
        return false;
    }
    return validarRegistro64(memoria.registroBase, "el registro base", numeroLinea, gestor);
}

static bool validarEtiqueta(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    if (instruccion.cantidadOperandos() > 0) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, instruccion.numeroLinea,
                            "las definiciones de etiqueta no admiten operandos");
        return false;
    }
    return true;
}

static bool validarRetorno(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    if (instruccion.cantidadOperandos() > 0) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, instruccion.numeroLinea,
                            "la instruccion 'ret' no admite operandos en este subconjunto");
        return false;
    }
    return true;
}

static bool validarDestinoEtiqueta(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    std::string mnemonico = codigoOperacionATexto(instruccion.codigoOperacion);
    int numeroLinea = instruccion.numeroLinea;
    if (instruccion.cantidadOperandos() < 1) {
        gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' requiere 1 operando (etiqueta destino)");
        return false;
    }
    if (instruccion.cantidadOperandos() > 1) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' solo admite 1 operando (etiqueta destino)");
        return false;
    }
    if (!instruccion.operando(0).esEtiqueta()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "el operando de '" + mnemonico + "' debe ser una etiqueta valida");
        return false;
    }
    return true;
}

static bool validarStack(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    std::string mnemonico = codigoOperacionATexto(instruccion.codigoOperacion);
    int numeroLinea = instruccion.numeroLinea;
    if (instruccion.cantidadOperandos() < 1) {
        gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' requiere 1 operando (registro de 64 bits)");
        return false;
    }
    if (instruccion.cantidadOperandos() > 1) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' solo admite 1 operando en este subconjunto");
        return false;
    }

    const Operando& operando = instruccion.operando(0);
    if (operando.esInmediato()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "la instruccion '" + mnemonico + "' no admite constantes inmediatas en este subconjunto");
        return false;
    }
    if (operando.esMemoria()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "la instruccion '" + mnemonico + "' no admite operandos de memoria en este subconjunto");
        return false;
    }
    if (operando.esRegistro()) {
        return validarRegistro64(operando.valor, "el registro", numeroLinea, gestor);
    }
    if (esSubregistroX86(operando.valor)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro '" + operando.valor + "' no soportado en modo solo 64 bits");
        return false;
    }
    gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                        "el operando de '" + mnemonico + "' debe ser un registro de 64 bits");
    return false;
}

static bool validarDestinoMovimiento(const Operando& destino, int numeroLinea, GestorDiagnosticos& gestor) {
    if (destino.esInmediato()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "el destino de 'mov' no puede ser una constante inmediata");
        return false;
    }
    if (destino.esRegistro()) {
        return validarRegistro64(destino.valor, "el registro destino", numeroLinea, gestor);
    }
    if (destino.esMemoria()) {
        return validarBaseMemoria(destino.memoria, numeroLinea, gestor);
    }
    if (esSubregistroX86(destino.valor)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro '" + destino.valor + "' no soportado en modo solo 64 bits");
        return false;
    }
    gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                        "el primer operando de 'mov' debe ser un registro de destino o una referencia a memoria");
    return false;
}

static bool validarFuenteMovimiento(const Operando& fuente, const Operando& destino,
                                    int numeroLinea, GestorDiagnosticos& gestor) {
    if (fuente.esRegistro()) {
        return validarRegistro64(fuente.valor, "el registro fuente", numeroLinea, gestor);
    }
    if (fuente.esInmediato()) {
        if (destino.esMemoria()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                                "no se admite almacenar una constante inmediata directamente en memoria en este subconjunto");
            return false;
        }
        return true;
    }
    if (fuente.esMemoria()) {
        if (destino.esMemoria()) {
            gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                                "no se admite transferencia directa de memoria a memoria en mov");
            return false;
        }
        return validarBaseMemoria(fuente.memoria, numeroLinea, gestor);
    }
    if (esSubregistroX86(fuente.valor)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro '" + fuente.valor + "' no soportado en modo solo 64 bits");
        return false;
    }
    gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                        "el operando fuente de 'mov' debe ser un registro, un valor inmediato o una referencia a memoria");
    return false;
}

static bool validarMovimiento(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    if (instruccion.cantidadOperandos() < 2) {
        gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, instruccion.numeroLinea,
                            "la instruccion 'mov' requiere 2 operandos (destino, fuente)");
        return false;
    }
    if (instruccion.cantidadOperandos() > 2) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, instruccion.numeroLinea,
                            "la instruccion 'mov' admite un maximo de 2 operandos en x86-64");
        return false;
    }

    const Operando& destino = instruccion.operando(0);
    const Operando& fuente = instruccion.operando(1);
    if (!validarDestinoMovimiento(destino, instruccion.numeroLinea, gestor)) {
        return false;
    }
    return validarFuenteMovimiento(fuente, destino, instruccion.numeroLinea, gestor);
}

static bool validarOperacionConDestinoRegistro(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    std::string mnemonico = codigoOperacionATexto(instruccion.codigoOperacion);
    int numeroLinea = instruccion.numeroLinea;
    if (instruccion.cantidadOperandos() < 2) {
        gestor.agregarError(CodigosDiagnostico::kFaltanOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' requiere 2 operandos (destino, fuente)");
        return false;
    }
    if (instruccion.cantidadOperandos() > 2) {
        gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, numeroLinea,
                            "la instruccion '" + mnemonico + "' admite un maximo de 2 operandos en x86-64");
        return false;
    }

    const Operando& destino = instruccion.operando(0);
    const Operando& fuente = instruccion.operando(1);
    if (destino.esMemoria() || fuente.esMemoria()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "la instruccion '" + mnemonico + "' no admite operandos de memoria en este subconjunto");
        return false;
    }
    if (destino.esInmediato()) {
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "el destino de '" + mnemonico + "' no puede ser una constante inmediata");
        return false;
    }
    if (!destino.esRegistro()) {
        if (esSubregistroX86(destino.valor)) {
            gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                                "registro '" + destino.valor + "' no soportado en modo solo 64 bits");
            return false;
        }
        gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                            "el primer operando de '" + mnemonico + "' debe ser un registro de destino");
        return false;
    }
    if (!validarRegistro64(destino.valor, "el registro destino", numeroLinea, gestor)) {
        return false;
    }
    if (fuente.esRegistro()) {
        return validarRegistro64(fuente.valor, "el registro fuente", numeroLinea, gestor);
    }
    if (fuente.esInmediato()) {
        return true;
    }
    if (esSubregistroX86(fuente.valor)) {
        gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, numeroLinea,
                            "registro '" + fuente.valor + "' no soportado en modo solo 64 bits");
        return false;
    }
    gestor.agregarError(CodigosDiagnostico::kFormaOperandoNoSoportada, numeroLinea,
                        "el operando fuente de '" + mnemonico + "' debe ser un registro o un valor inmediato");
    return false;
}

bool Validador::validarInstruccion(const Instruccion& instruccion, GestorDiagnosticos& gestor) {
    if (instruccion.errorSintactico.has_value()) {
        gestor.agregar(*instruccion.errorSintactico);
        return false;
    }

    switch (instruccion.codigoOperacion) {
        case CodigoOperacion::Etiqueta:
            return validarEtiqueta(instruccion, gestor);
        case CodigoOperacion::Ret:
            return validarRetorno(instruccion, gestor);
        case CodigoOperacion::Jmp:
        case CodigoOperacion::Je:
        case CodigoOperacion::Jne:
        case CodigoOperacion::Jl:
        case CodigoOperacion::Jle:
        case CodigoOperacion::Jg:
        case CodigoOperacion::Jge:
        case CodigoOperacion::Call:
            return validarDestinoEtiqueta(instruccion, gestor);
        case CodigoOperacion::Push:
        case CodigoOperacion::Pop:
            return validarStack(instruccion, gestor);
        case CodigoOperacion::Mov:
            return validarMovimiento(instruccion, gestor);
        case CodigoOperacion::Add:
        case CodigoOperacion::Sub:
        case CodigoOperacion::And:
        case CodigoOperacion::Or:
        case CodigoOperacion::Xor:
        case CodigoOperacion::Cmp:
            return validarOperacionConDestinoRegistro(instruccion, gestor);
        case CodigoOperacion::Desconocido:
            gestor.agregarError(CodigosDiagnostico::kInstruccionDesconocida, instruccion.numeroLinea,
                                "instruccion no soportada o desconocida");
            return false;
        default:
            gestor.agregarError(CodigosDiagnostico::kInstruccionDesconocida, instruccion.numeroLinea,
                                "forma de instruccion no soportada");
            return false;
    }
}

bool Validador::validarPrograma(const std::vector<Instruccion>& instrucciones, GestorDiagnosticos& gestor) {
    std::unordered_set<std::string> etiquetasDefinidas;
    for (const auto& instruccion : instrucciones) {
        if (instruccion.tieneEtiqueta()) {
            etiquetasDefinidas.insert(instruccion.etiqueta);
        }
    }

    bool todoValido = true;
    for (const auto& instruccion : instrucciones) {
        if (!validarInstruccion(instruccion, gestor)) {
            todoValido = false;
            continue;
        }
        if (!instruccion.esSalto() && !instruccion.esLlamada()) {
            continue;
        }

        const Operando& destino = instruccion.operando(0);
        if (etiquetasDefinidas.find(destino.valor) == etiquetasDefinidas.end()) {
            std::string tipoDestino = "salto";
            if (instruccion.esLlamada()) {
                tipoDestino = "llamada";
            }
            gestor.agregarError(CodigosDiagnostico::kEtiquetaNoDefinida, instruccion.numeroLinea,
                                "etiqueta de " + tipoDestino + " '" + destino.valor + "' no definida en el programa");
            todoValido = false;
        }
    }
    return todoValido;
}
