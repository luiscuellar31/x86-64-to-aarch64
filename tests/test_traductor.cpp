#include <iostream>
#include <cassert>
#include "traductor.hpp"

void probarTraduccionMovimiento() {
    std::cout << "[Test] Probando traduccion de movimiento de datos (mov)...\n";

    std::string codigo =
        "mov rax, 42\n"
        "mov rbx, rax\n"
        "ret\n";

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());
    assert(!res.tieneErrores());
    assert(res.mapeos.size() == 3);

    // Verificamos que las instrucciones AArch64 emitidas son las correctas
    assert(res.codigoGenerado.find("mov x0, #42") != std::string::npos);
    assert(res.codigoGenerado.find("mov x19, x0") != std::string::npos);
    assert(res.codigoGenerado.find("ret") != std::string::npos);

    // Verificamos mapeos y reglas didácticas
    assert(res.mapeos[0].reglaId == "MOV_REG_IMM");
    assert(res.mapeos[1].reglaId == "MOV_REG_REG");
    assert(res.mapeos[2].reglaId == "RET_SIMPLE");

    std::cout << "  -> Exito en traduccion de mov y ret.\n";
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

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());

    // Verificamos que add y sub tienen 3 operandos en AArch64
    assert(res.codigoGenerado.find("add x0, x0, #25") != std::string::npos);
    assert(res.codigoGenerado.find("sub x0, x0, #5") != std::string::npos);
    assert(res.codigoGenerado.find("add x0, x0, x19") != std::string::npos);
    assert(res.codigoGenerado.find("sub x0, x0, x19") != std::string::npos);

    assert(res.mapeos[1].reglaId == "ARITH_ADD_IMM");
    assert(res.mapeos[2].reglaId == "ARITH_SUB_IMM");
    assert(res.mapeos[3].reglaId == "ARITH_ADD_REG");
    assert(res.mapeos[4].reglaId == "ARITH_SUB_REG");

    std::cout << "  -> Exito en traduccion de add y sub con 3 operandos.\n";
}

void probarTraduccionConEtiquetas() {
    std::cout << "[Test] Probando programa completo con etiquetas...\n";

    std::string codigo =
        "inicio:\n"
        "    mov rax, 10\n"
        "    add rax, 5\n"
        "    ret\n";

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());

    assert(res.codigoGenerado.find("inicio:") != std::string::npos);
    assert(res.codigoGenerado.find("mov x0, #10") != std::string::npos);
    assert(res.codigoGenerado.find("add x0, x0, #5") != std::string::npos);

    std::cout << "  -> Exito en traduccion con etiquetas.\n";
}

void probarRechazoProgramasInvalidos() {
    std::cout << "[Test] Probando rechazo de programas invalidos y emision de diagnosticos...\n";

    // 1. Destino inmediato (invertido)
    std::string codigoInvertido = "mov 42, rax\nret\n";
    ResultadoTraduccion res1 = Traductor::traducir(codigoInvertido);
    assert(!res1.esExitoso());
    assert(res1.tieneErrores());
    assert(res1.diagnosticos[0].codigo == CodigosDiagnostico::kFormaOperandoNoSoportada);

    // 2. Registro no soportado de 32 bits
    std::string codigoReg32 = "mov eax, 10\nret\n";
    ResultadoTraduccion res2 = Traductor::traducir(codigoReg32);
    assert(!res2.esExitoso());
    assert(res2.diagnosticos[0].codigo == CodigosDiagnostico::kRegistroNoSoportado64Bit);

    // 3. Instruccion desconocida
    std::string codigoDesc = "vmovaps ymm0, ymm1\nret\n";
    ResultadoTraduccion res3 = Traductor::traducir(codigoDesc);
    assert(!res3.esExitoso());
    assert(res3.diagnosticos[0].codigo == CodigosDiagnostico::kInstruccionDesconocida);

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

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());
    assert(res.mapeos.size() == 7);

    // Verificamos traducciones correctas en AArch64 (and, orr, eor)
    assert(res.codigoGenerado.find("and x0, x0, x19") != std::string::npos);
    assert(res.codigoGenerado.find("and x0, x0, #15") != std::string::npos);
    assert(res.codigoGenerado.find("orr x19, x19, x4") != std::string::npos);
    assert(res.codigoGenerado.find("orr x19, x19, #1") != std::string::npos);
    assert(res.codigoGenerado.find("eor x0, x0, x0") != std::string::npos);
    assert(res.codigoGenerado.find("eor x0, x0, #255") != std::string::npos);

    // Verificamos IDs de reglas
    assert(res.mapeos[0].reglaId == "LOGIC_AND_REG");
    assert(res.mapeos[1].reglaId == "LOGIC_AND_IMM");
    assert(res.mapeos[2].reglaId == "LOGIC_OR_REG");
    assert(res.mapeos[3].reglaId == "LOGIC_OR_IMM");
    assert(res.mapeos[4].reglaId == "LOGIC_XOR_REG");
    assert(res.mapeos[5].reglaId == "LOGIC_XOR_IMM");
    assert(res.mapeos[6].reglaId == "RET_SIMPLE");

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

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());
    assert(!res.tieneErrores());

    // Verificamos instrucciones emitidas en AArch64
    assert(res.codigoGenerado.find("cmp x0, x19") != std::string::npos);
    assert(res.codigoGenerado.find("b.eq fin") != std::string::npos);
    assert(res.codigoGenerado.find("cmp x0, #10") != std::string::npos);
    assert(res.codigoGenerado.find("b.ne etiqueta1") != std::string::npos);
    assert(res.codigoGenerado.find("b.lt etiqueta2") != std::string::npos);
    assert(res.codigoGenerado.find("b.le etiqueta3") != std::string::npos);
    assert(res.codigoGenerado.find("b.gt etiqueta4") != std::string::npos);
    assert(res.codigoGenerado.find("b.ge etiqueta5") != std::string::npos);
    assert(res.codigoGenerado.find("b fin") != std::string::npos);
    assert(res.codigoGenerado.find("ret") != std::string::npos);

    // Verificamos reglas didacticas
    bool tieneCmpReg = false, tieneCmpImm = false, tieneB = false, tieneBEq = false;
    for (const auto& mapeo : res.mapeos) {
        if (mapeo.reglaId == "COND_CMP_REG") tieneCmpReg = true;
        if (mapeo.reglaId == "COND_CMP_IMM") tieneCmpImm = true;
        if (mapeo.reglaId == "JUMP_UNCOND") tieneB = true;
        if (mapeo.reglaId == "JUMP_COND_EQ") tieneBEq = true;
    }
    assert(tieneCmpReg && tieneCmpImm && tieneB && tieneBEq);

    std::cout << "  -> Exito en traduccion de cmp y saltos (jmp->b, je->b.eq, etc.).\n";
}

void probarTraduccionMemoriaLocal() {
    std::cout << "[Test] Probando traduccion de memoria local (mov con stack -> ldr/str)...\n";

    std::string codigo =
        "mov rax, [rbp - 8]\n"
        "mov [rbp - 16], rbx\n"
        "mov rcx, [rbp + 16]\n"
        "mov [rbp], rdx\n"
        "ret\n";

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());
    assert(!res.tieneErrores());
    assert(res.mapeos.size() == 5);

    // 1. Carga con desplazamiento negativo: ldr x0, [x29, #-8]
    assert(res.codigoGenerado.find("ldr x0, [x29, #-8]") != std::string::npos);
    assert(res.mapeos[0].reglaId == "MEM_LOAD_STACK");

    // 2. Almacenamiento con desplazamiento negativo: str x19, [x29, #-16]
    assert(res.codigoGenerado.find("str x19, [x29, #-16]") != std::string::npos);
    assert(res.mapeos[1].reglaId == "MEM_STORE_STACK");

    // 3. Carga con desplazamiento positivo: ldr x4, [x29, #16]
    assert(res.codigoGenerado.find("ldr x4, [x29, #16]") != std::string::npos);
    assert(res.mapeos[2].reglaId == "MEM_LOAD_STACK");

    // 4. Almacenamiento con desplazamiento cero: str x3, [x29]
    assert(res.codigoGenerado.find("str x3, [x29]") != std::string::npos);
    assert(res.mapeos[3].reglaId == "MEM_STORE_STACK");

    // 5. Retorno
    assert(res.codigoGenerado.find("ret") != std::string::npos);
    assert(res.mapeos[4].reglaId == "RET_SIMPLE");

    std::cout << "  -> Exito en traduccion de memoria local (ldr/str) a AArch64.\n";
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

    ResultadoTraduccion res1 = Traductor::traducir(codigoPrologo);
    assert(res1.esExitoso());
    assert(!res1.tieneErrores());

    assert(res1.codigoGenerado.find("str x29, [sp, #-16]!") != std::string::npos);
    assert(res1.codigoGenerado.find("mov x29, sp") != std::string::npos);
    assert(res1.codigoGenerado.find("mov x0, #42") != std::string::npos);
    assert(res1.codigoGenerado.find("mov sp, x29") != std::string::npos);
    assert(res1.codigoGenerado.find("ldr x29, [sp], #16") != std::string::npos);
    assert(res1.codigoGenerado.find("ret") != std::string::npos);

    // 2. Preservacion de registro callee-saved (rbx -> x19)
    std::string codigoCallee =
        "push rbx\n"
        "mov rbx, 10\n"
        "pop rbx\n"
        "ret\n";

    ResultadoTraduccion res2 = Traductor::traducir(codigoCallee);
    assert(res2.esExitoso());
    assert(res2.codigoGenerado.find("str x19, [sp, #-16]!") != std::string::npos);
    assert(res2.codigoGenerado.find("mov x19, #10") != std::string::npos);
    assert(res2.codigoGenerado.find("ldr x19, [sp], #16") != std::string::npos);

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

    ResultadoTraduccion res = Traductor::traducir(codigo);
    assert(res.esExitoso());
    assert(!res.tieneErrores());

    // Verificamos que se genera 'bl duplicar' en AArch64
    assert(res.codigoGenerado.find("bl duplicar") != std::string::npos);
    assert(res.codigoGenerado.find("ret") != std::string::npos);
    assert(res.codigoGenerado.find("duplicar:") != std::string::npos);

    // Verificamos la regla pedagogica
    bool tieneCallDirect = false;
    for (const auto& mapeo : res.mapeos) {
        if (mapeo.reglaId == "CALL_DIRECT") {
            tieneCallDirect = true;
            assert(mapeo.codigoOrigen == "call duplicar");
            assert(mapeo.codigoDestino == "bl duplicar");
        }
    }
    assert(tieneCallDirect);

    std::cout << "  -> Exito en traduccion de llamadas a funcion (call -> bl).\n";
}

int main() {
    std::cout << "=== Pruebas de Traduccion de Extremo a Extremo ===\n";
    probarTraduccionMovimiento();
    probarTraduccionAritmetica();
    probarTraduccionLogica();
    probarTraduccionComparacionYSaltos();
    probarTraduccionMemoriaLocal();
    probarTraduccionStackYPrologoEpilogo();
    probarTraduccionLlamadas();
    probarTraduccionConEtiquetas();
    probarRechazoProgramasInvalidos();
    std::cout << "Todas las pruebas del traductor pasaron con exito.\n";
    return 0;
}
