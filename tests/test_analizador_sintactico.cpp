#include <iostream>
#include <cassert>
#include "analizador_sintactico.hpp"
#include "traductor.hpp"
#include <limits>

void probarAnalisisInstruccionesBasicas() {
    std::cout << "[Test] Probando analisis de instrucciones basicas (mov, add, sub, ret)...\n";

    // 1. add rax, 5
    Instruccion instruccionAdd = AnalizadorSintactico::analizarTexto("add rax, 5", 4);
    assert(instruccionAdd.codigoOperacion == CodigoOperacion::Add);
    assert(instruccionAdd.numeroLinea == 4);
    assert(instruccionAdd.operandos.size() == 2);
    assert(instruccionAdd.operandos[0].tipo == TipoOperando::Registro);
    assert(instruccionAdd.operandos[0].valor == "rax");
    assert(instruccionAdd.operandos[1].tipo == TipoOperando::Inmediato);
    assert(instruccionAdd.operandos[1].valor == "5");

    // 2. mov rbx, rax
    Instruccion instruccionMov = AnalizadorSintactico::analizarTexto("MOV RBX, RAX", 5);
    assert(instruccionMov.codigoOperacion == CodigoOperacion::Mov);
    assert(instruccionMov.operandos.size() == 2);
    assert(instruccionMov.operandos[0].tipo == TipoOperando::Registro);
    assert(instruccionMov.operandos[0].valor == "rbx");
    assert(instruccionMov.operandos[1].tipo == TipoOperando::Registro);
    assert(instruccionMov.operandos[1].valor == "rax");

    // 3. sub rax, 0x10
    Instruccion instruccionSub = AnalizadorSintactico::analizarTexto("sub rax, 0x10", 6);
    assert(instruccionSub.codigoOperacion == CodigoOperacion::Sub);
    assert(instruccionSub.operandos.size() == 2);
    assert(instruccionSub.operandos[1].tipo == TipoOperando::Inmediato);
    assert(instruccionSub.operandos[1].valor == "0x10");

    // 4. ret
    Instruccion instruccionRet = AnalizadorSintactico::analizarTexto("  RET  ", 7);
    assert(instruccionRet.codigoOperacion == CodigoOperacion::Ret);
    assert(instruccionRet.operandos.empty());

    std::cout << "  -> Exito en analisis de instrucciones basicas.\n";
}

void probarAnalisisEtiquetas() {
    std::cout << "[Test] Probando analisis de etiquetas y combinaciones...\n";

    // 1. Etiqueta pura
    Instruccion instruccionEtiqueta = AnalizadorSintactico::analizarTexto("inicio:", 1);
    assert(instruccionEtiqueta.tieneEtiqueta());
    assert(instruccionEtiqueta.esEtiquetaPura());
    assert(instruccionEtiqueta.etiqueta == "inicio");

    // 2. Etiqueta con instruccion acompañante
    Instruccion instruccionCombinada = AnalizadorSintactico::analizarTexto("bucle: ADD RAX, 1", 10);
    assert(instruccionCombinada.tieneEtiqueta());
    assert(instruccionCombinada.etiqueta == "bucle");
    assert(instruccionCombinada.codigoOperacion == CodigoOperacion::Add);
    assert(instruccionCombinada.operandos.size() == 2);
    assert(instruccionCombinada.operandos[0].valor == "rax");
    assert(instruccionCombinada.operandos[1].valor == "1");

    std::cout << "  -> Exito en analisis de etiquetas.\n";
}

void probarAnalisisProgramaCompleto() {
    std::cout << "[Test] Probando analisis de programa completo...\n";

    std::string codigo =
        "; Prueba de programa\n"
        "inicio:\n"
        "    mov rax, 10\n"
        "    add rax, 5\n"
        "    ret\n";

    std::vector<Instruccion> programa = AnalizadorSintactico::analizarPrograma(codigo);
    assert(programa.size() == 4);

    assert(programa[0].esEtiquetaPura());
    assert(programa[0].etiqueta == "inicio");
    assert(programa[0].numeroLinea == 2);

    assert(programa[1].codigoOperacion == CodigoOperacion::Mov);
    assert(programa[1].numeroLinea == 3);

    assert(programa[2].codigoOperacion == CodigoOperacion::Add);
    assert(programa[2].numeroLinea == 4);

    assert(programa[3].codigoOperacion == CodigoOperacion::Ret);
    assert(programa[3].numeroLinea == 5);

    std::cout << "  -> Exito en analisis de programa completo.\n";
}

void probarAnalisisOperandosMemoria() {
    std::cout << "[Test] Probando analisis de operandos de memoria estructurados...\n";

    // 1. mov rax, [rbp - 8]
    Instruccion instruccionLoad = AnalizadorSintactico::analizarTexto("mov rax, [rbp - 8]", 1);
    assert(instruccionLoad.codigoOperacion == CodigoOperacion::Mov);
    assert(instruccionLoad.operandos.size() == 2);
    assert(instruccionLoad.operandos[0].esRegistro() && instruccionLoad.operandos[0].valor == "rax");
    assert(instruccionLoad.operandos[1].esMemoria());
    assert(instruccionLoad.operandos[1].memoria.registroBase == "rbp");
    assert(instruccionLoad.operandos[1].memoria.desplazamiento == -8);
    assert(instruccionLoad.operandos[1].memoria.anchoBits == 64);

    // 2. mov [rbp - 16], rbx
    Instruccion instruccionStore = AnalizadorSintactico::analizarTexto("mov [rbp - 16], rbx", 2);
    assert(instruccionStore.codigoOperacion == CodigoOperacion::Mov);
    assert(instruccionStore.operandos.size() == 2);
    assert(instruccionStore.operandos[0].esMemoria());
    assert(instruccionStore.operandos[0].memoria.registroBase == "rbp");
    assert(instruccionStore.operandos[0].memoria.desplazamiento == -16);
    assert(instruccionStore.operandos[1].esRegistro() && instruccionStore.operandos[1].valor == "rbx");

    // 3. mov rax, [rbp + 32]
    Instruccion instruccionPositivo = AnalizadorSintactico::analizarTexto("mov rax, [rbp + 32]", 3);
    assert(instruccionPositivo.operandos[1].esMemoria());
    assert(instruccionPositivo.operandos[1].memoria.desplazamiento == 32);

    // 4. mov rax, [rbp] (desplazamiento cero)
    Instruccion instruccionCero = AnalizadorSintactico::analizarTexto("mov rax, [rbp]", 4);
    assert(instruccionCero.operandos[1].esMemoria());
    assert(instruccionCero.operandos[1].memoria.desplazamiento == 0);

    // 5. mov rax, [rbp-8] (sin espacios)
    Instruccion instruccionCompacto = AnalizadorSintactico::analizarTexto("mov rax, [rbp-8]", 5);
    assert(instruccionCompacto.operandos[1].esMemoria());
    assert(instruccionCompacto.operandos[1].memoria.desplazamiento == -8);

    // 6. clasificarOperando individual
    Operando opMem = AnalizadorSintactico::clasificarOperando("[rbp - 24]");
    assert(opMem.esMemoria());
    assert(opMem.memoria.registroBase == "rbp");
    assert(opMem.memoria.desplazamiento == -24);

    std::cout << "  -> Exito en analisis de operandos de memoria.\n";
}

void probarAnalisisStack() {
    std::cout << "[Test] Probando analisis de instrucciones de stack (push, pop)...\n";

    // 1. push rbp
    Instruccion instruccionPush = AnalizadorSintactico::analizarTexto("push rbp", 1);
    assert(instruccionPush.codigoOperacion == CodigoOperacion::Push);
    assert(instruccionPush.esStack());
    assert(instruccionPush.operandos.size() == 1);
    assert(instruccionPush.operandos[0].esRegistro());
    assert(instruccionPush.operandos[0].valor == "rbp");

    // 2. pop rbp
    Instruccion instruccionPop = AnalizadorSintactico::analizarTexto("pop rbp", 2);
    assert(instruccionPop.codigoOperacion == CodigoOperacion::Pop);
    assert(instruccionPop.esStack());
    assert(instruccionPop.operandos.size() == 1);
    assert(instruccionPop.operandos[0].esRegistro());
    assert(instruccionPop.operandos[0].valor == "rbp");

    // 3. push rbx y pop rbx
    Instruccion instruccionPushRbx = AnalizadorSintactico::analizarTexto("push rbx", 3);
    assert(instruccionPushRbx.codigoOperacion == CodigoOperacion::Push);
    assert(instruccionPushRbx.operandos[0].valor == "rbx");

    Instruccion instruccionPopRbx = AnalizadorSintactico::analizarTexto("pop rbx", 4);
    assert(instruccionPopRbx.codigoOperacion == CodigoOperacion::Pop);
    assert(instruccionPopRbx.operandos[0].valor == "rbx");

    std::cout << "  -> Exito en analisis de push y pop.\n";
}

void probarAnalisisLlamadas() {
    std::cout << "[Test] Probando analisis de instruccion call...\n";

    Instruccion instruccionCall = AnalizadorSintactico::analizarTexto("call calcular_suma", 1);
    assert(instruccionCall.codigoOperacion == CodigoOperacion::Call);
    assert(instruccionCall.esLlamada());
    assert(instruccionCall.operandos.size() == 1);
    assert(instruccionCall.operandos[0].esEtiqueta());
    assert(instruccionCall.operandos[0].valor == "calcular_suma");

    std::cout << "  -> Exito en analisis de call.\n";
}

void probarRechazoSintaxisMalformada() {
    const std::vector<std::string> casos = {
        "add rax 5", "add ,rax, 5", "add rax,,5", "add rax, 5,",
        "add rax,", "ret ,", "ret , ,", "push ,rax", "mov,rax,1",
        "mov[rbp],rax", "mov rax, [rbp 8]", "mov rax, [rbp - -8]",
        "mov rax, [rbp + +8]", "mov rax, [rbp +]", "mov rax, [rbp - 8",
        "mov rax, [rbp - 8]]", "mov rax, [rbp, 8]", "mov rax, []",
        "mov rax, [rbp + rax*4]", "mov rax, 12abc", "mov rax, 0x1g",
        "9inicio: ret", "call destino,\ndestino:\nret"
    };

    for (const auto& texto : casos) {
        ResultadoTraduccion resultado = Traductor::traducir("; cabecera\n\n" + texto + "\n");
        assert(!resultado.esExitoso());
        assert(resultado.codigoGenerado.empty());
        assert(resultado.irDestino.empty());
        assert(resultado.mapeos.empty());
        assert(!resultado.diagnosticos.empty());
        assert(resultado.diagnosticos[0].codigo == CodigosDiagnostico::kSintaxisInvalida);
        assert(resultado.diagnosticos[0].numeroLinea == 3);
    }
}

void probarLimitesNumericosYConsumoCompleto() {
    const std::vector<std::string> invalidos = {
        "0x", "+0x", "-0x", "9223372036854775808",
        "-9223372036854775809", "0x8000000000000000"
    };
    for (const auto& literal : invalidos) {
        assert(!Operando::crearInmediato(literal).esInmediato());
        ResultadoTraduccion resultado = Traductor::traducir("mov rax, " + literal);
        assert(!resultado.esExitoso());
        assert(resultado.codigoGenerado.empty());
        assert(resultado.diagnosticos[0].codigo == CodigosDiagnostico::kInmediatoInvalido);
    }

    const std::vector<std::string> operandosInvalidos = {
        "rax rbx", "10 basura", "0x1g", "12abc", "[rbp] basura", "[rbp 8]",
        "[rbp - 9223372036854775809]", "[rbp+0x8000000000000000]"
    };
    for (const auto& texto : operandosInvalidos) {
        assert(AnalizadorSintactico::clasificarOperando(texto).tipo == TipoOperando::Desconocido);
    }

    assert(Operando::crearInmediato("010").valorNumerico == 10);
    assert(Operando::crearInmediato("08").valorNumerico == 8);
    assert(Operando::crearInmediato("-0x8000000000000000").valorNumerico == std::numeric_limits<int64_t>::min());
    assert(Operando::crearInmediato("9223372036854775807").valorNumerico == std::numeric_limits<int64_t>::max());
    Operando minimo = AnalizadorSintactico::clasificarOperando("[rbp - 9223372036854775808]");
    assert(minimo.esMemoria());
    assert(minimo.memoria.desplazamiento == std::numeric_limits<int64_t>::min());
    assert(minimo.memoria.aTexto() == "[rbp - 9223372036854775808]");

    for (const auto& texto : {"[RBP-08]", "[rbp - 8]", "[rbp- 8]", "[rbp -8]"}) {
        Operando memoria = AnalizadorSintactico::clasificarOperando(texto);
        assert(memoria.esMemoria());
        assert(memoria.memoria.registroBase == "rbp");
        assert(memoria.memoria.desplazamiento == -8);
    }

    const std::vector<std::string> desplazamientosInvalidos = {
        "[rbp - 9223372036854775809]", "[rbp+0x8000000000000000]"
    };
    for (const auto& memoria : desplazamientosInvalidos) {
        ResultadoTraduccion resultado = Traductor::traducir("mov rax, " + memoria);
        assert(!resultado.esExitoso());
        assert(resultado.codigoGenerado.empty());
    }
}

void probarAvanceDelIndiceDeMemoria() {
    struct CasoMemoria {
        std::string texto;
        size_t indiceEsperado;
        bool esValido;
    };
    const std::vector<CasoMemoria> casos = {
        {"[rbp - 8], rax", 5, true},
        {"[rbp-8], rax", 4, true},
        {"[rbp 8], rax", 4, false},
        {"[rbp + +8], rax", 5, false},
        {"[rbp - 8", 4, false},
        {"[], rax", 2, false}
    };
    for (const auto& caso : casos) {
        std::vector<Token> tokens = AnalizadorLexico::tokenizarLinea(caso.texto, 5);
        size_t indice = 0;
        Operando operando = AnalizadorSintactico::analizarOperandoMemoria(tokens, indice);
        assert(indice == caso.indiceEsperado);
        assert(operando.esMemoria() == caso.esValido);
    }

    std::vector<Token> tokens = AnalizadorLexico::tokenizarPrograma("[rbp +\nret\n");
    size_t indice = 0;
    Operando invalido = AnalizadorSintactico::analizarOperandoMemoria(tokens, indice);
    assert(invalido.tipo == TipoOperando::Desconocido);
    assert(indice == 3);
    assert(tokens[indice].es(TipoToken::FinDeLinea));
}

int main() {
    std::cout << "=== Pruebas unitarias de AnalizadorSintactico ===\n";
    probarAnalisisInstruccionesBasicas();
    probarAnalisisEtiquetas();
    probarAnalisisProgramaCompleto();
    probarAnalisisOperandosMemoria();
    probarAnalisisStack();
    probarAnalisisLlamadas();
    probarRechazoSintaxisMalformada();
    probarLimitesNumericosYConsumoCompleto();
    probarAvanceDelIndiceDeMemoria();
    std::cout << "Todas las pruebas del analizador sintactico pasaron con exito.\n";
    return 0;
}
