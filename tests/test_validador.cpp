#include <iostream>
#include <cassert>
#include "validador.hpp"
#include "analizador_sintactico.hpp"

void probarCasosValidos() {
    std::cout << "[Test Gate] Probando instrucciones validas...\n";

    GestorDiagnosticos gestor;
    std::vector<Instruccion> validas = {
        AnalizadorSintactico::analizarTexto("mov rax, rbx", 1),
        AnalizadorSintactico::analizarTexto("mov rax, 42", 2),
        AnalizadorSintactico::analizarTexto("add rax, rbx", 3),
        AnalizadorSintactico::analizarTexto("add rax, 5", 4),
        AnalizadorSintactico::analizarTexto("sub rax, 1", 5),
        AnalizadorSintactico::analizarTexto("ret", 6),
        AnalizadorSintactico::analizarTexto("inicio:", 7),
        AnalizadorSintactico::analizarTexto("and rax, rbx", 8),
        AnalizadorSintactico::analizarTexto("and rax, 15", 9),
        AnalizadorSintactico::analizarTexto("or rbx, rcx", 10),
        AnalizadorSintactico::analizarTexto("or rbx, 1", 11),
        AnalizadorSintactico::analizarTexto("xor rax, rax", 12),
        AnalizadorSintactico::analizarTexto("xor rax, 255", 13),
        AnalizadorSintactico::analizarTexto("cmp rax, rbx", 14),
        AnalizadorSintactico::analizarTexto("cmp rax, 0", 15),
        AnalizadorSintactico::analizarTexto("je fin", 16),
        AnalizadorSintactico::analizarTexto("jne fin", 17),
        AnalizadorSintactico::analizarTexto("jl fin", 18),
        AnalizadorSintactico::analizarTexto("jle fin", 19),
        AnalizadorSintactico::analizarTexto("jg fin", 20),
        AnalizadorSintactico::analizarTexto("jge fin", 21),
        AnalizadorSintactico::analizarTexto("jmp fin", 22),
        AnalizadorSintactico::analizarTexto("fin:", 23)
    };

    bool resultado = Validador::validarPrograma(validas, gestor);
    assert(resultado == true);
    assert(!gestor.tieneErrores());
    assert(gestor.cantidadErrores() == 0);

    std::cout << "  -> Exito en casos validos.\n";
}

void probarRegistrosDesconocidos() {
    std::cout << "[Test Gate] Probando rechazo de registros no soportados o de 32 bits (E021)...\n";

    GestorDiagnosticos gestor;
    // 'eax' no es de 64 bits en nuestro subconjunto
    Instruccion instruccionReg32 = AnalizadorSintactico::analizarTexto("mov eax, 10", 1);
    bool resultado = Validador::validarInstruccion(instruccionReg32, gestor);

    assert(resultado == false);
    assert(gestor.tieneErrores());
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    // Fuente con registro no soportado
    GestorDiagnosticos gestor2;
    Instruccion instruccionFuenteInvalida = AnalizadorSintactico::analizarTexto("add rax, r99", 2);
    assert(Validador::validarInstruccion(instruccionFuenteInvalida, gestor2) == false);

    // cmp con subregistro
    GestorDiagnosticos gestorCmp;
    Instruccion instruccionCmp32 = AnalizadorSintactico::analizarTexto("cmp eax, 0", 3);
    assert(Validador::validarInstruccion(instruccionCmp32, gestorCmp) == false);
    assert(gestorCmp.obtenerTodos()[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    std::cout << "  -> Exito en rechazo de registros desconocidos.\n";
}

void probarOperandosInvertidos() {
    std::cout << "[Test Gate] Probando rechazo de operandos invertidos o de tipo invalido (E014)...\n";

    GestorDiagnosticos gestor;
    // Constante como primer operando (destino inmediato)
    Instruccion instruccionInvertida = AnalizadorSintactico::analizarTexto("mov 42, rax", 3);
    bool resultado = Validador::validarInstruccion(instruccionInvertida, gestor);

    assert(resultado == false);
    assert(gestor.tieneErrores());
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // Destino inmediato en operacion logica
    GestorDiagnosticos gestorXor;
    Instruccion instruccionXorInvertida = AnalizadorSintactico::analizarTexto("xor 42, rax", 4);
    assert(Validador::validarInstruccion(instruccionXorInvertida, gestorXor) == false);
    assert(gestorXor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // Primer operando inmediato en cmp
    GestorDiagnosticos gestorCmp;
    Instruccion instruccionCmpInvertida = AnalizadorSintactico::analizarTexto("cmp 10, rax", 5);
    assert(Validador::validarInstruccion(instruccionCmpInvertida, gestorCmp) == false);
    assert(gestorCmp.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // Salto con operando numerico inmediato en vez de etiqueta
    GestorDiagnosticos gestorJmp;
    Instruccion instruccionJmpImm = AnalizadorSintactico::analizarTexto("jmp 42", 6);
    assert(Validador::validarInstruccion(instruccionJmpImm, gestorJmp) == false);
    assert(gestorJmp.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    std::cout << "  -> Exito en rechazo de operandos invertidos e invalidos.\n";
}

void probarExcesoOperandos() {
    std::cout << "[Test Gate] Probando rechazo de exceso de operandos (E022)...\n";

    // 1. Tres operandos en add
    GestorDiagnosticos gestor1;
    Instruccion instruccionTresOps = AnalizadorSintactico::analizarTexto("add rax, rbx, rcx", 4);
    assert(Validador::validarInstruccion(instruccionTresOps, gestor1) == false);
    assert(gestor1.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 2. Operando en ret
    GestorDiagnosticos gestor2;
    Instruccion instruccionRetConOp = AnalizadorSintactico::analizarTexto("ret rax", 5);
    assert(Validador::validarInstruccion(instruccionRetConOp, gestor2) == false);
    assert(gestor2.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 3. Tres operandos en cmp
    GestorDiagnosticos gestor3;
    Instruccion instruccionCmpTres = AnalizadorSintactico::analizarTexto("cmp rax, rbx, 5", 6);
    assert(Validador::validarInstruccion(instruccionCmpTres, gestor3) == false);
    assert(gestor3.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 4. Dos operandos en jmp
    GestorDiagnosticos gestor4;
    Instruccion instruccionJmpDos = AnalizadorSintactico::analizarTexto("jmp fin, rax", 7);
    assert(Validador::validarInstruccion(instruccionJmpDos, gestor4) == false);
    assert(gestor4.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    std::cout << "  -> Exito en rechazo de exceso de operandos.\n";
}

void probarFaltaOperandos() {
    std::cout << "[Test Gate] Probando rechazo por falta de operandos (E023)...\n";

    GestorDiagnosticos gestor;
    Instruccion instruccionUnOp = AnalizadorSintactico::analizarTexto("mov rax", 6);
    assert(Validador::validarInstruccion(instruccionUnOp, gestor) == false);
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    // cmp sin segundo operando
    GestorDiagnosticos gestorCmp;
    Instruccion instruccionCmpUno = AnalizadorSintactico::analizarTexto("cmp rax", 7);
    assert(Validador::validarInstruccion(instruccionCmpUno, gestorCmp) == false);
    assert(gestorCmp.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    // jmp sin operando
    GestorDiagnosticos gestorJmp;
    Instruccion instruccionJmpCero = AnalizadorSintactico::analizarTexto("jmp", 8);
    assert(Validador::validarInstruccion(instruccionJmpCero, gestorJmp) == false);
    assert(gestorJmp.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    std::cout << "  -> Exito en rechazo por falta de operandos.\n";
}

void probarInstruccionDesconocida() {
    std::cout << "[Test Gate] Probando rechazo de instruccion desconocida (E001)...\n";

    GestorDiagnosticos gestor;
    Instruccion instruccionDesconocida = AnalizadorSintactico::analizarTexto("vmovaps ymm0, ymm1", 7);
    assert(Validador::validarInstruccion(instruccionDesconocida, gestor) == false);
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kInstruccionDesconocida);

    std::cout << "  -> Exito en rechazo de instruccion desconocida.\n";
}

void probarEtiquetasNoDefinidas() {
    std::cout << "[Test Gate] Probando rechazo de saltos a etiquetas no definidas (E040)...\n";

    GestorDiagnosticos gestor;
    std::vector<Instruccion> programa = {
        AnalizadorSintactico::analizarTexto("inicio:", 1),
        AnalizadorSintactico::analizarTexto("cmp rax, 0", 2),
        AnalizadorSintactico::analizarTexto("je destino_fantasma", 3),
        AnalizadorSintactico::analizarTexto("ret", 4)
    };

    bool resultado = Validador::validarPrograma(programa, gestor);
    assert(resultado == false);
    assert(gestor.tieneErrores());
    assert(gestor.obtenerTodos()[0].codigo == CodigosDiagnostico::kEtiquetaNoDefinida);

    std::cout << "  -> Exito en rechazo de salto a etiqueta no definida.\n";
}

void probarMemoriaLocal() {
    std::cout << "[Test Gate] Probando validacion de memoria local controlada...\n";

    // 1. Casos validos de memoria en mov
    GestorDiagnosticos gestorValido;
    std::vector<Instruccion> validas = {
        AnalizadorSintactico::analizarTexto("mov rax, [rbp - 8]", 1),
        AnalizadorSintactico::analizarTexto("mov [rbp - 16], rbx", 2),
        AnalizadorSintactico::analizarTexto("mov rax, [rbp + 16]", 3),
        AnalizadorSintactico::analizarTexto("mov rax, [rbp]", 4),
        AnalizadorSintactico::analizarTexto("mov [rsp], rax", 5)
    };
    assert(Validador::validarPrograma(validas, gestorValido) == true);
    assert(!gestorValido.tieneErrores());

    // 2. Rechazo de memoria a memoria
    GestorDiagnosticos gestorMemMem;
    Instruccion instruccionMemMem = AnalizadorSintactico::analizarTexto("mov [rbp - 8], [rbp - 16]", 6);
    assert(Validador::validarInstruccion(instruccionMemMem, gestorMemMem) == false);
    assert(gestorMemMem.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 3. Rechazo de inmediato a memoria
    GestorDiagnosticos gestorMemImm;
    Instruccion instruccionMemImm = AnalizadorSintactico::analizarTexto("mov [rbp - 8], 42", 7);
    assert(Validador::validarInstruccion(instruccionMemImm, gestorMemImm) == false);
    assert(gestorMemImm.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 4. Rechazo de registro base de 32 bits (ebp)
    GestorDiagnosticos gestorBase32;
    Instruccion instruccionBase32 = AnalizadorSintactico::analizarTexto("mov rax, [ebp - 8]", 8);
    assert(Validador::validarInstruccion(instruccionBase32, gestorBase32) == false);
    assert(gestorBase32.obtenerTodos()[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);
    assert(gestorBase32.obtenerTodos()[0].formatear() ==
           "linea 8: [E021] ERROR: registro base 'ebp' no soportado en modo solo 64 bits");

    GestorDiagnosticos gestorDestino32;
    Instruccion destino32 = AnalizadorSintactico::analizarTexto("mov [ebp], 42", 9);
    assert(!Validador::validarInstruccion(destino32, gestorDestino32));
    assert(gestorDestino32.obtenerTodos()[0].formatear() ==
           "linea 9: [E021] ERROR: registro base 'ebp' no soportado en modo solo 64 bits");

    // 5. Rechazo de memoria en operacion aritmetica (add)
    GestorDiagnosticos gestorAddMem;
    Instruccion instruccionAddMem = AnalizadorSintactico::analizarTexto("add rax, [rbp - 8]", 9);
    assert(Validador::validarInstruccion(instruccionAddMem, gestorAddMem) == false);
    assert(gestorAddMem.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 6. Rechazo de memoria en operacion logica (xor)
    GestorDiagnosticos gestorXorMem;
    Instruccion instruccionXorMem = AnalizadorSintactico::analizarTexto("xor [rbp - 8], rax", 10);
    assert(Validador::validarInstruccion(instruccionXorMem, gestorXorMem) == false);
    assert(gestorXorMem.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 7. Rechazo de memoria en comparacion (cmp)
    GestorDiagnosticos gestorCmpMem;
    Instruccion instruccionCmpMem = AnalizadorSintactico::analizarTexto("cmp rax, [rbp - 8]", 11);
    assert(Validador::validarInstruccion(instruccionCmpMem, gestorCmpMem) == false);
    assert(gestorCmpMem.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    std::cout << "  -> Exito en validacion de memoria local.\n";
}

void probarInstruccionesStack() {
    std::cout << "[Test Gate] Probando validacion de instrucciones de stack (push, pop)...\n";

    // 1. Casos validos
    GestorDiagnosticos gestorValido;
    std::vector<Instruccion> validas = {
        AnalizadorSintactico::analizarTexto("push rbp", 1),
        AnalizadorSintactico::analizarTexto("push rbx", 2),
        AnalizadorSintactico::analizarTexto("pop rbx", 3),
        AnalizadorSintactico::analizarTexto("pop rbp", 4)
    };
    assert(Validador::validarPrograma(validas, gestorValido) == true);
    assert(!gestorValido.tieneErrores());

    // 2. Falta de operandos
    GestorDiagnosticos gestorFaltaPush, gestorFaltaPop;
    Instruccion instruccionPushVacia = AnalizadorSintactico::analizarTexto("push", 5);
    Instruccion instruccionPopVacia = AnalizadorSintactico::analizarTexto("pop", 6);
    assert(Validador::validarInstruccion(instruccionPushVacia, gestorFaltaPush) == false);
    assert(gestorFaltaPush.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);
    assert(Validador::validarInstruccion(instruccionPopVacia, gestorFaltaPop) == false);
    assert(gestorFaltaPop.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    // 3. Exceso de operandos
    GestorDiagnosticos gestorExceso;
    Instruccion instruccionExceso = AnalizadorSintactico::analizarTexto("push rax, rbx", 7);
    assert(Validador::validarInstruccion(instruccionExceso, gestorExceso) == false);
    assert(gestorExceso.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 4. Inmediato no permitido en este subconjunto
    GestorDiagnosticos gestorImm;
    Instruccion instruccionPushImm = AnalizadorSintactico::analizarTexto("push 42", 8);
    assert(Validador::validarInstruccion(instruccionPushImm, gestorImm) == false);
    assert(gestorImm.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 5. Subregistros de 32 bits
    GestorDiagnosticos gestor32;
    Instruccion instruccionPush32 = AnalizadorSintactico::analizarTexto("push ebp", 9);
    assert(Validador::validarInstruccion(instruccionPush32, gestor32) == false);
    assert(gestor32.obtenerTodos()[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    // 6. Memoria no permitida en push/pop
    GestorDiagnosticos gestorMem;
    Instruccion instruccionPushMem = AnalizadorSintactico::analizarTexto("push [rbp - 8]", 10);
    assert(Validador::validarInstruccion(instruccionPushMem, gestorMem) == false);
    assert(gestorMem.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    std::cout << "  -> Exito en validacion de push y pop.\n";
}

void probarValidacionLlamadas() {
    std::cout << "[Test Gate] Probando validacion de instruccion call...\n";

    // 1. Caso valido con definicion de subrutina
    GestorDiagnosticos gestorValido;
    std::vector<Instruccion> programa = {
        AnalizadorSintactico::analizarTexto("inicio:", 1),
        AnalizadorSintactico::analizarTexto("call duplicar", 2),
        AnalizadorSintactico::analizarTexto("ret", 3),
        AnalizadorSintactico::analizarTexto("duplicar:", 4),
        AnalizadorSintactico::analizarTexto("ret", 5)
    };
    assert(Validador::validarPrograma(programa, gestorValido) == true);
    assert(!gestorValido.tieneErrores());

    // 2. Falta de operando (call sin destino)
    GestorDiagnosticos gestorFalta;
    Instruccion instruccionCallVacia = AnalizadorSintactico::analizarTexto("call", 6);
    assert(Validador::validarInstruccion(instruccionCallVacia, gestorFalta) == false);
    assert(gestorFalta.obtenerTodos()[0].codigo == CodigosDiagnostico::kFaltanOperandos);

    // 3. Exceso de operandos (call con mas de una etiqueta)
    GestorDiagnosticos gestorExceso;
    Instruccion instruccionCallExceso = AnalizadorSintactico::analizarTexto("call f1, f2", 7);
    assert(Validador::validarInstruccion(instruccionCallExceso, gestorExceso) == false);
    assert(gestorExceso.obtenerTodos()[0].codigo == CodigosDiagnostico::kExcesoOperandos);

    // 4. Operando de llamada no es etiqueta (inmediato o registro indirecto no soportado)
    GestorDiagnosticos gestorImm;
    Instruccion instruccionCallImm = AnalizadorSintactico::analizarTexto("call 42", 8);
    assert(Validador::validarInstruccion(instruccionCallImm, gestorImm) == false);
    assert(gestorImm.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    GestorDiagnosticos gestorReg;
    Instruccion instruccionCallReg = AnalizadorSintactico::analizarTexto("call rax", 9);
    assert(Validador::validarInstruccion(instruccionCallReg, gestorReg) == false);
    assert(gestorReg.obtenerTodos()[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 5. Etiqueta de llamada no definida en el programa
    GestorDiagnosticos gestorNoDef;
    std::vector<Instruccion> progInexistente = {
        AnalizadorSintactico::analizarTexto("inicio:", 1),
        AnalizadorSintactico::analizarTexto("call subrutina_fantasma", 2),
        AnalizadorSintactico::analizarTexto("ret", 3)
    };
    assert(Validador::validarPrograma(progInexistente, gestorNoDef) == false);
    assert(gestorNoDef.obtenerTodos()[0].codigo == CodigosDiagnostico::kEtiquetaNoDefinida);

    std::cout << "  -> Exito en validacion de call.\n";
}

int main() {
    std::cout << "=== Pruebas Gate del Validador Semantico ===\n";
    probarCasosValidos();
    probarRegistrosDesconocidos();
    probarOperandosInvertidos();
    probarExcesoOperandos();
    probarFaltaOperandos();
    probarInstruccionDesconocida();
    probarEtiquetasNoDefinidas();
    probarMemoriaLocal();
    probarInstruccionesStack();
    probarValidacionLlamadas();
    std::cout << "Todas las condiciones Gate del validador pasaron con exito.\n";
    return 0;
}
