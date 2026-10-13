#include <iostream>
#include <cassert>
#include "traductor.hpp"

void probarTraduccionMovimiento() {
    std::cout << "[Test] Probando traduccion de movimiento de datos (mov)...\n";

    std::string codigo =
        "mov rax, 42\n"
        "mov rbx, rax\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(!resultado.tieneErrores());
    assert(resultado.mapeos.size() == 3);

    assert(resultado.codigoGenerado ==
        "    mov x0, #42\n"
        "    mov x19, x0\n"
        "    ret\n");

    // Verificamos mapeos y reglas didácticas
    assert(resultado.mapeos[0].reglaId == "MOV_REG_IMM");
    assert(resultado.mapeos[1].reglaId == "MOV_REG_REG");
    assert(resultado.mapeos[2].reglaId == "RET_SIMPLE");

    std::cout << "  -> Exito en traduccion de mov y ret.\n";
}

void probarMapeoDeTodosLosRegistros() {
    std::cout << "[Test] Probando el mapeo de todos los registros de 64 bits...\n";

    std::string codigo =
        "mov rax, rdi\n"
        "mov rdi, rsi\n"
        "mov rsi, rdx\n"
        "mov rdx, rcx\n"
        "mov rcx, r8\n"
        "mov r8, r9\n"
        "mov r9, r10\n"
        "mov r10, r11\n"
        "mov r11, rbx\n"
        "mov rbx, r12\n"
        "mov r12, r13\n"
        "mov r13, r14\n"
        "mov r14, r15\n"
        "mov r15, rbp\n"
        "mov rbp, rsp\n"
        "mov rsp, rbp\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.codigoGenerado ==
        "    mov x0, x1\n"
        "    mov x1, x2\n"
        "    mov x2, x3\n"
        "    mov x3, x4\n"
        "    mov x4, x5\n"
        "    mov x5, x6\n"
        "    mov x6, x7\n"
        "    mov x7, x8\n"
        "    mov x8, x19\n"
        "    mov x19, x20\n"
        "    mov x20, x21\n"
        "    mov x21, x22\n"
        "    mov x22, x23\n"
        "    mov x23, x29\n"
        "    mov x29, sp\n"
        "    mov sp, x29\n"
        "    ret\n");
}

void probarTraduccionInmediatos() {
    std::cout << "[Test] Probando inmediatos negativos y hexadecimales...\n";

    std::string codigo =
        "mov rax, -1\n"
        "mov rbx, 0x2a\n"
        "add rax, 0x10\n"
        "sub rax, 0x8\n"
        "and rbx, 0xff\n"
        "or rbx, 0x1\n"
        "xor rbx, 0xff\n"
        "cmp rax, 0x10\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.codigoGenerado ==
        "    mov x0, #-1\n"
        "    mov x19, #0x2a\n"
        "    add x0, x0, #0x10\n"
        "    sub x0, x0, #0x8\n"
        "    and x19, x19, #0xff\n"
        "    orr x19, x19, #0x1\n"
        "    eor x19, x19, #0xff\n"
        "    cmp x0, #0x10\n"
        "    ret\n");
}

void probarTraduccionAritmetica() {
    std::cout << "[Test] Probando traduccion de operaciones aritmeticas (add, sub)...\n";

    std::string codigo =
        "mov rax, 100\n"
        "add rax, 25\n"
        "sub rax, 5\n"
        "add rax, rbx\n"
        "sub rax, rbx\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());

    assert(resultado.codigoGenerado ==
        "    mov x0, #100\n"
        "    add x0, x0, #25\n"
        "    sub x0, x0, #5\n"
        "    add x0, x0, x19\n"
        "    sub x0, x0, x19\n"
        "    ret\n");

    assert(resultado.mapeos[1].reglaId == "ARITH_ADD_IMM");
    assert(resultado.mapeos[2].reglaId == "ARITH_SUB_IMM");
    assert(resultado.mapeos[3].reglaId == "ARITH_ADD_REG");
    assert(resultado.mapeos[4].reglaId == "ARITH_SUB_REG");

    std::cout << "  -> Exito en traduccion de add y sub con 3 operandos.\n";
}

void probarTraduccionSinInstrucciones() {
    std::cout << "[Test] Probando entradas vacias y comentarios sin instrucciones...\n";

    ResultadoTraduccion resultadoVacio = Traductor::traducir("");
    assert(resultadoVacio.esExitoso());
    assert(resultadoVacio.codigoGenerado == "");
    assert(resultadoVacio.irDestino.empty());
    assert(resultadoVacio.mapeos.empty());

    ResultadoTraduccion resultadoComentarios = Traductor::traducir("; Cabecera\n\n  ; Comentario final");
    assert(resultadoComentarios.esExitoso());
    assert(resultadoComentarios.codigoGenerado == "");
    assert(resultadoComentarios.irDestino.empty());
    assert(resultadoComentarios.mapeos.empty());
}

void probarTraduccionConEtiquetas() {
    std::cout << "[Test] Probando programa completo con etiquetas...\n";

    std::string codigo =
        "; Programa con etiquetas que distinguen mayusculas\n"
        "\n"
        "Inicio:\n"
        "    MOV RAX, 10 ; valor inicial\n"
        "    jmp Fin\n"
        "\n"
        "inicio: ADD RAX, 5\n"
        "Fin: RET";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());

    assert(resultado.codigoGenerado ==
        "Inicio:\n"
        "    mov x0, #10\n"
        "    b Fin\n"
        "    inicio: add x0, x0, #5\n"
        "    Fin: ret\n");

    assert(resultado.mapeos.size() == 5);
    assert(resultado.mapeos[0].lineaOrigenX86 == 3);
    assert(resultado.mapeos[1].lineaOrigenX86 == 4);
    assert(resultado.mapeos[2].lineaOrigenX86 == 5);
    assert(resultado.mapeos[3].lineaOrigenX86 == 7);
    assert(resultado.mapeos[4].lineaOrigenX86 == 8);
    assert(resultado.mapeos[3].codigoOrigen == "inicio: add rax, 5");
    assert(resultado.mapeos[3].codigoDestino == "inicio: add x0, x0, #5");
    assert(resultado.mapeos[3].lineasDestinoAArch64 == std::vector<int>({4}));
    assert(resultado.mapeos[4].lineasDestinoAArch64 == std::vector<int>({5}));

    std::cout << "  -> Exito en traduccion con etiquetas.\n";
}

void probarRechazoProgramasInvalidos() {
    std::cout << "[Test] Probando rechazo de programas invalidos y emision de diagnosticos...\n";

    // 1. Destino inmediato (invertido)
    std::string codigoInvertido = "mov 42, rax\nret\n";
    ResultadoTraduccion resultadoDestinoInvertido = Traductor::traducir(codigoInvertido);
    assert(!resultadoDestinoInvertido.esExitoso());
    assert(resultadoDestinoInvertido.tieneErrores());
    assert(resultadoDestinoInvertido.diagnosticos[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 2. Registro no soportado de 32 bits
    std::string codigoReg32 = "mov eax, 10\nret\n";
    ResultadoTraduccion resultadoRegistro32Bits = Traductor::traducir(codigoReg32);
    assert(!resultadoRegistro32Bits.esExitoso());
    assert(resultadoRegistro32Bits.diagnosticos[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    // 3. Instruccion desconocida
    std::string codigoDesc = "vmovaps ymm0, ymm1\nret\n";
    ResultadoTraduccion resultadoInstruccionDesconocida = Traductor::traducir(codigoDesc);
    assert(!resultadoInstruccionDesconocida.esExitoso());
    assert(resultadoInstruccionDesconocida.diagnosticos[0].codigo == CodigosDiagnostico::kInstruccionDesconocida);

    std::cout << "  -> Exito en rechazo y diagnosticos de casos invalidos.\n";
}

void probarTraduccionLogica() {
    std::cout << "[Test] Probando traduccion de operaciones logicas (and, or, xor)...\n";

    std::string codigo =
        "and rax, rbx\n"
        "and rax, 15\n"
        "or rbx, rcx\n"
        "or rbx, 1\n"
        "xor rax, rax\n"
        "xor rax, 255\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.mapeos.size() == 7);

    assert(resultado.codigoGenerado ==
        "    and x0, x0, x19\n"
        "    and x0, x0, #15\n"
        "    orr x19, x19, x4\n"
        "    orr x19, x19, #1\n"
        "    eor x0, x0, x0\n"
        "    eor x0, x0, #255\n"
        "    ret\n");

    // Verificamos IDs de reglas
    assert(resultado.mapeos[0].reglaId == "LOGIC_AND_REG");
    assert(resultado.mapeos[1].reglaId == "LOGIC_AND_IMM");
    assert(resultado.mapeos[2].reglaId == "LOGIC_OR_REG");
    assert(resultado.mapeos[3].reglaId == "LOGIC_OR_IMM");
    assert(resultado.mapeos[4].reglaId == "LOGIC_XOR_REG");
    assert(resultado.mapeos[5].reglaId == "LOGIC_XOR_IMM");
    assert(resultado.mapeos[6].reglaId == "RET_SIMPLE");

    std::cout << "  -> Exito en traduccion de operaciones logicas (and -> and, or -> orr, xor -> eor).\n";
}

void probarTraduccionComparacionYSaltos() {
    std::cout << "[Test] Probando traduccion de comparacion (cmp) y control de flujo...\n";

    std::string codigo =
        "inicio:\n"
        "    cmp rax, rbx\n"
        "    je fin\n"
        "    cmp rax, 10\n"
        "    jne etiqueta1\n"
        "etiqueta1:\n"
        "    jl etiqueta2\n"
        "etiqueta2:\n"
        "    jle etiqueta3\n"
        "etiqueta3:\n"
        "    jg etiqueta4\n"
        "etiqueta4:\n"
        "    jge etiqueta5\n"
        "etiqueta5:\n"
        "    jmp fin\n"
        "fin:\n"
        "    ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(!resultado.tieneErrores());

    assert(resultado.codigoGenerado ==
        "inicio:\n"
        "    cmp x0, x19\n"
        "    b.eq fin\n"
        "    cmp x0, #10\n"
        "    b.ne etiqueta1\n"
        "etiqueta1:\n"
        "    b.lt etiqueta2\n"
        "etiqueta2:\n"
        "    b.le etiqueta3\n"
        "etiqueta3:\n"
        "    b.gt etiqueta4\n"
        "etiqueta4:\n"
        "    b.ge etiqueta5\n"
        "etiqueta5:\n"
        "    b fin\n"
        "fin:\n"
        "    ret\n");

    assert(resultado.mapeos.size() == 17);
    assert(resultado.mapeos[1].reglaId == "COND_CMP_REG");
    assert(resultado.mapeos[2].reglaId == "JUMP_COND_EQ");
    assert(resultado.mapeos[3].reglaId == "COND_CMP_IMM");
    assert(resultado.mapeos[4].reglaId == "JUMP_COND_NE");
    assert(resultado.mapeos[6].reglaId == "JUMP_COND_LT");
    assert(resultado.mapeos[8].reglaId == "JUMP_COND_LE");
    assert(resultado.mapeos[10].reglaId == "JUMP_COND_GT");
    assert(resultado.mapeos[12].reglaId == "JUMP_COND_GE");
    assert(resultado.mapeos[14].reglaId == "JUMP_UNCOND");

    std::cout << "  -> Exito en traduccion de cmp y saltos (jmp->b, je->b.eq, etc.).\n";
}

void probarTraduccionMemoriaLocal() {
    std::cout << "[Test] Probando traduccion de memoria local (mov con stack -> ldr/str)...\n";

    std::string codigo =
        "mov rax, [rbp - 8]\n"
        "mov [rbp - 16], rbx\n"
        "mov rcx, [rbp + 16]\n"
        "mov [rbp], rdx\n"
        "mov r8, [rbp]\n"
        "mov [rbp + 8], r9\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(!resultado.tieneErrores());
    assert(resultado.mapeos.size() == 7);

    assert(resultado.codigoGenerado ==
        "    ldr x0, [x29, #-8]\n"
        "    str x19, [x29, #-16]\n"
        "    ldr x4, [x29, #16]\n"
        "    str x3, [x29]\n"
        "    ldr x5, [x29]\n"
        "    str x6, [x29, #8]\n"
        "    ret\n");

    assert(resultado.mapeos[0].reglaId == "MEM_LOAD_STACK");
    assert(resultado.mapeos[1].reglaId == "MEM_STORE_STACK");
    assert(resultado.mapeos[2].reglaId == "MEM_LOAD_STACK");
    assert(resultado.mapeos[3].reglaId == "MEM_STORE_STACK");
    assert(resultado.mapeos[4].reglaId == "MEM_LOAD_STACK");
    assert(resultado.mapeos[5].reglaId == "MEM_STORE_STACK");
    assert(resultado.mapeos[6].reglaId == "RET_SIMPLE");

    std::cout << "  -> Exito en traduccion de memoria local (ldr/str) a AArch64.\n";
}

void probarTraduccionMemoriaConBaseRsp() {
    std::cout << "[Test] Probando memoria con base rsp y desplazamientos negativos, positivos y cero...\n";

    std::string codigo =
        "mov rax, [rsp - 8]\n"
        "mov [rsp - 16], rbx\n"
        "mov rcx, [rsp + 16]\n"
        "mov [rsp], rdx\n"
        "mov r8, [rsp]\n"
        "mov [rsp + 8], r9\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.codigoGenerado ==
        "    ldr x0, [sp, #-8]\n"
        "    str x19, [sp, #-16]\n"
        "    ldr x4, [sp, #16]\n"
        "    str x3, [sp]\n"
        "    ldr x5, [sp]\n"
        "    str x6, [sp, #8]\n"
        "    ret\n");
}

void probarTraduccionAritmeticaYComparacionConRsp() {
    std::cout << "[Test] Probando ajustes de pila y comparaciones con rsp...\n";

    std::string codigo =
        "mov rax, 16\n"
        "sub rsp, 32\n"
        "add rsp, rax\n"
        "sub rsp, rax\n"
        "add rsp, 32\n"
        "cmp rsp, rax\n"
        "cmp rsp, 0\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.codigoGenerado ==
        "    mov x0, #16\n"
        "    sub sp, sp, #32\n"
        "    add sp, sp, x0\n"
        "    sub sp, sp, x0\n"
        "    add sp, sp, #32\n"
        "    cmp sp, x0\n"
        "    cmp sp, #0\n"
        "    ret\n");
}

void probarTraduccionStackYPrologoEpilogo() {
    std::cout << "[Test] Probando traduccion de stack, prologo y epilogo (push/pop)...\n";

    // 1. Prologo y epilogo canonico con rbp y rsp
    std::string codigoPrologo =
        "inicio:\n"
        "    push rbp\n"
        "    mov rbp, rsp\n"
        "    mov rax, 42\n"
        "    mov rsp, rbp\n"
        "    pop rbp\n"
        "    ret\n";

    ResultadoTraduccion resultadoPrologo = Traductor::traducir(codigoPrologo);
    assert(resultadoPrologo.esExitoso());
    assert(!resultadoPrologo.tieneErrores());
    assert(resultadoPrologo.codigoGenerado ==
        "inicio:\n"
        "    str x29, [sp, #-16]!\n"
        "    mov x29, sp\n"
        "    mov x0, #42\n"
        "    mov sp, x29\n"
        "    ldr x29, [sp], #16\n"
        "    ret\n");
    assert(resultadoPrologo.mapeos.size() == 7);
    assert(resultadoPrologo.mapeos[1].reglaId == "STACK_PUSH_REG");
    assert(resultadoPrologo.mapeos[5].reglaId == "STACK_POP_REG");

    // 2. Preservacion de registro callee-saved (rbx -> x19)
    std::string codigoCallee =
        "push rbx\n"
        "mov rbx, 10\n"
        "pop rbx\n"
        "ret\n";

    ResultadoTraduccion resultadoPreservacion = Traductor::traducir(codigoCallee);
    assert(resultadoPreservacion.esExitoso());
    assert(resultadoPreservacion.codigoGenerado ==
        "    str x19, [sp, #-16]!\n"
        "    mov x19, #10\n"
        "    ldr x19, [sp], #16\n"
        "    ret\n");
    assert(resultadoPreservacion.mapeos.size() == 4);
    assert(resultadoPreservacion.mapeos[0].reglaId == "STACK_PUSH_REG");
    assert(resultadoPreservacion.mapeos[2].reglaId == "STACK_POP_REG");

    std::cout << "  -> Exito en traduccion de prologo, epilogo y stack (push/pop).\n";
}

void probarTraduccionLlamadas() {
    std::cout << "[Test] Probando traduccion de llamadas a funcion (call -> bl)...\n";

    std::string codigo =
        "inicio:\n"
        "    mov rdi, 10\n"
        "    call duplicar\n"
        "    ret\n"
        "\n"
        "duplicar:\n"
        "    add rdi, rdi\n"
        "    mov rax, rdi\n"
        "    ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(!resultado.tieneErrores());

    assert(resultado.codigoGenerado ==
        "inicio:\n"
        "    mov x1, #10\n"
        "    str x30, [sp, #-16]!\n"
        "    bl duplicar\n"
        "    ldr x30, [sp], #16\n"
        "    ret\n"
        "duplicar:\n"
        "    add x1, x1, x1\n"
        "    mov x0, x1\n"
        "    ret\n");

    // Verificamos la regla pedagogica
    bool tieneCallDirect = false;
    for (const auto& mapeo : resultado.mapeos) {
        if (mapeo.reglaId == "CALL_DIRECT") {
            tieneCallDirect = true;
            assert(mapeo.codigoOrigen == "call duplicar");
            assert(mapeo.codigoDestino == "str x30, [sp, #-16]!\nbl duplicar\nldr x30, [sp], #16");
            assert(mapeo.lineasDestinoAArch64 == std::vector<int>({3, 4, 5}));
        }
    }
    assert(tieneCallDirect);

    std::cout << "  -> Exito en traduccion de llamadas a funcion (call -> bl).\n";
}

void probarLlamadasSucesivasYAnidadas() {
    std::string codigo =
        "inicio:\n"
        "mov rax, 7\n"
        "call duplicar\n"
        "call incrementar\n"
        "ret\n"
        "duplicar:\n"
        "push rbp\n"
        "mov rbp, rsp\n"
        "call doblar\n"
        "mov rsp, rbp\n"
        "pop rbp\n"
        "ret\n"
        "doblar: add rax, rax\n"
        "ret\n"
        "incrementar: add rax, 1\n"
        "ret\n";

    ResultadoTraduccion resultado = Traductor::traducir(codigo);
    assert(resultado.esExitoso());
    assert(resultado.codigoGenerado ==
        "inicio:\n"
        "    mov x0, #7\n"
        "    str x30, [sp, #-16]!\n"
        "    bl duplicar\n"
        "    ldr x30, [sp], #16\n"
        "    str x30, [sp, #-16]!\n"
        "    bl incrementar\n"
        "    ldr x30, [sp], #16\n"
        "    ret\n"
        "duplicar:\n"
        "    str x29, [sp, #-16]!\n"
        "    mov x29, sp\n"
        "    str x30, [sp, #-16]!\n"
        "    bl doblar\n"
        "    ldr x30, [sp], #16\n"
        "    mov sp, x29\n"
        "    ldr x29, [sp], #16\n"
        "    ret\n"
        "    doblar: add x0, x0, x0\n"
        "    ret\n"
        "    incrementar: add x0, x0, #1\n"
        "    ret\n");

    assert(resultado.mapeos.size() == 16);
    assert(resultado.irDestino.size() == 22);
    assert(resultado.mapeos[2].lineasDestinoAArch64 == std::vector<int>({3, 4, 5}));
    assert(resultado.mapeos[3].lineasDestinoAArch64 == std::vector<int>({6, 7, 8}));
    assert(resultado.mapeos[8].lineasDestinoAArch64 == std::vector<int>({13, 14, 15}));
    assert(resultado.mapeos[12].codigoOrigen == "doblar: add rax, rax");
    assert(resultado.mapeos[12].codigoDestino == "doblar: add x0, x0, x0");

    for (size_t indice : {2, 5, 12}) {
        int lineaOrigen = resultado.irDestino[indice].lineaOrigen;
        for (size_t desplazamiento = 0; desplazamiento < 3; desplazamiento++) {
            assert(resultado.irDestino[indice + desplazamiento].lineaOrigen == lineaOrigen);
            assert(resultado.irDestino[indice + desplazamiento].reglaId == "CALL_DIRECT");
            assert(!resultado.irDestino[indice + desplazamiento].explicacion.empty());
        }
    }

    ResultadoTraduccion hoja = Traductor::traducir("mov rax, 42\nret\n");
    assert(hoja.codigoGenerado == "    mov x0, #42\n    ret\n");
    ResultadoTraduccion rechazada = Traductor::traducir("call ausente\nret\n");
    assert(!rechazada.esExitoso());
    assert(rechazada.codigoGenerado.empty());
    assert(rechazada.irDestino.empty());
    assert(rechazada.mapeos.empty());
}

void probarSecuenciaDeLlamadaConEtiqueta() {
    Instruccion llamada(CodigoOperacion::Call, {Operando::crearEtiqueta("destino")}, 4);
    llamada.etiqueta = "repetir";
    MapeoTraduccion mapeo;
    std::vector<InstruccionAArch64> secuencia = Traductor::traducirInstruccion(llamada, mapeo);
    assert(secuencia.size() == 3);
    assert(secuencia[0].emitirTexto() == "repetir: str x30, [sp, #-16]!");
    assert(secuencia[1].emitirTexto() == "bl destino");
    assert(secuencia[2].emitirTexto() == "ldr x30, [sp], #16");
    assert(mapeo.codigoOrigen == "repetir: call destino");
    assert(mapeo.codigoDestino == "repetir: str x30, [sp, #-16]!\nbl destino\nldr x30, [sp], #16");
    for (const auto& instruccion : secuencia) {
        assert(instruccion.lineaOrigen == 4);
        assert(instruccion.reglaId == "CALL_DIRECT");
    }

    ResultadoTraduccion programa = Traductor::traducir("repetir: call destino\nret\ndestino: ret\n");
    assert(programa.esExitoso());
    assert(programa.codigoGenerado ==
        "    repetir: str x30, [sp, #-16]!\n"
        "    bl destino\n"
        "    ldr x30, [sp], #16\n"
        "    ret\n"
        "    destino: ret\n");
    assert(programa.mapeos[0].lineasDestinoAArch64 == std::vector<int>({1, 2, 3}));
    assert(programa.mapeos[2].lineasDestinoAArch64 == std::vector<int>({5}));
}

int main() {
    std::cout << "=== Pruebas de Traduccion de Extremo a Extremo ===\n";
    probarTraduccionMovimiento();
    probarMapeoDeTodosLosRegistros();
    probarTraduccionInmediatos();
    probarTraduccionAritmetica();
    probarTraduccionLogica();
    probarTraduccionComparacionYSaltos();
    probarTraduccionMemoriaLocal();
    probarTraduccionMemoriaConBaseRsp();
    probarTraduccionAritmeticaYComparacionConRsp();
    probarTraduccionStackYPrologoEpilogo();
    probarTraduccionLlamadas();
    probarLlamadasSucesivasYAnidadas();
    probarSecuenciaDeLlamadaConEtiqueta();
    probarTraduccionConEtiquetas();
    probarTraduccionSinInstrucciones();
    probarRechazoProgramasInvalidos();
    std::cout << "Todas las pruebas del traductor pasaron con exito.\n";
    return 0;
}
