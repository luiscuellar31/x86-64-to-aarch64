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

int main() {
    std::cout << "=== Pruebas de Traduccion de Extremo a Extremo ===\n";
    probarTraduccionMovimiento();
    probarTraduccionAritmetica();
    probarTraduccionLogica();
    probarTraduccionConEtiquetas();
    probarRechazoProgramasInvalidos();
    std::cout << "Todas las pruebas del traductor pasaron con exito.\n";
    return 0;
}
