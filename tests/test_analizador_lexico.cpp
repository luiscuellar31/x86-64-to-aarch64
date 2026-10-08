#include <iostream>
#include <cassert>
#include "analizador_lexico.hpp"

void probarTokensIndividuales() {
    std::cout << "[Test] Probando clasificacion de tokens individuales...\n";

    // 1. Registros
    std::vector<Token> tokensReg = AnalizadorLexico::tokenizarLinea("rax rbx rsp", 1);
    assert(tokensReg.size() == 3);
    assert(tokensReg[0].es(TipoToken::Registro) && tokensReg[0].texto == "rax");
    assert(tokensReg[1].es(TipoToken::Registro) && tokensReg[1].texto == "rbx");
    assert(tokensReg[2].es(TipoToken::Registro) && tokensReg[2].texto == "rsp");

    // 2. Inmediatos (decimales, con signo, hexadecimales)
    std::vector<Token> tokensImm = AnalizadorLexico::tokenizarLinea("42 -10 +5 0x1F", 2);
    assert(tokensImm.size() == 4);
    assert(tokensImm[0].es(TipoToken::Inmediato) && tokensImm[0].texto == "42");
    assert(tokensImm[1].es(TipoToken::Inmediato) && tokensImm[1].texto == "-10");
    assert(tokensImm[2].es(TipoToken::Inmediato) && tokensImm[2].texto == "+5");
    assert(tokensImm[3].es(TipoToken::Inmediato) && tokensImm[3].texto == "0x1F");

    // 3. Puntuacion
    std::vector<Token> tokensPunt = AnalizadorLexico::tokenizarLinea(", :", 3);
    assert(tokensPunt.size() == 2);
    assert(tokensPunt[0].es(TipoToken::Coma));
    assert(tokensPunt[1].es(TipoToken::DosPuntos));

    std::cout << "  -> Exito en tokens individuales.\n";
}

void probarInstruccionTokenizada() {
    std::cout << "[Test] Probando tokenizacion de linea de instruccion completa...\n";

    std::vector<Token> tokens = AnalizadorLexico::tokenizarLinea("    add rax, 5 ; incremento", 4);
    assert(tokens.size() == 4);
    assert(tokens[0].es(TipoToken::Identificador) && tokens[0].texto == "add");
    assert(tokens[1].es(TipoToken::Registro) && tokens[1].texto == "rax");
    assert(tokens[2].es(TipoToken::Coma));
    assert(tokens[3].es(TipoToken::Inmediato) && tokens[3].texto == "5");

    std::cout << "  -> Exito en instruccion completa.\n";
}

void probarEtiquetasYPrograma() {
    std::cout << "[Test] Probando tokenizacion de etiquetas y programa...\n";

    std::string codigo =
        "inicio:\n"
        "    ret\n";

    std::vector<Token> tokens = AnalizadorLexico::tokenizarPrograma(codigo);
    // Linea 1: inicio, :, \n (FinDeLinea) -> 3 tokens
    // Linea 2: ret, \n (FinDeLinea) -> 2 tokens
    assert(tokens.size() == 5);
    assert(tokens[0].es(TipoToken::Identificador) && tokens[0].texto == "inicio");
    assert(tokens[1].es(TipoToken::DosPuntos));
    assert(tokens[2].es(TipoToken::FinDeLinea));
    assert(tokens[3].es(TipoToken::Identificador) && tokens[3].texto == "ret");
    assert(tokens[4].es(TipoToken::FinDeLinea));

    std::cout << "  -> Exito en programa con FinDeLinea.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de AnalizadorLexico (Fase 2, Subfase 3) ===\n";
    probarTokensIndividuales();
    probarInstruccionTokenizada();
    probarEtiquetasYPrograma();
    std::cout << "Todas las pruebas del analizador lexico pasaron con exito.\n";
    return 0;
}
