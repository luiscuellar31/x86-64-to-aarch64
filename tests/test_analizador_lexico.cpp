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

void probarTokensMemoria() {
    std::cout << "[Test] Probando tokenizacion de corchetes y operandos de memoria...\n";

    // 1. [rbp - 8] con espacios
    std::vector<Token> tokensEspacios = AnalizadorLexico::tokenizarLinea("[rbp - 8]", 5);
    assert(tokensEspacios.size() == 5);
    assert(tokensEspacios[0].es(TipoToken::CorcheteAbre));
    assert(tokensEspacios[1].es(TipoToken::Registro) && tokensEspacios[1].texto == "rbp");
    assert(tokensEspacios[2].es(TipoToken::Menos));
    assert(tokensEspacios[3].es(TipoToken::Inmediato) && tokensEspacios[3].texto == "8");
    assert(tokensEspacios[4].es(TipoToken::CorcheteCierra));

    // 2. [rbp + 16] con espacios
    std::vector<Token> tokensSuma = AnalizadorLexico::tokenizarLinea("[rbp + 16]", 6);
    assert(tokensSuma.size() == 5);
    assert(tokensSuma[0].es(TipoToken::CorcheteAbre));
    assert(tokensSuma[1].es(TipoToken::Registro) && tokensSuma[1].texto == "rbp");
    assert(tokensSuma[2].es(TipoToken::Mas));
    assert(tokensSuma[3].es(TipoToken::Inmediato) && tokensSuma[3].texto == "16");
    assert(tokensSuma[4].es(TipoToken::CorcheteCierra));

    // 3. [rbp] simple
    std::vector<Token> tokensSimple = AnalizadorLexico::tokenizarLinea("[rbp]", 7);
    assert(tokensSimple.size() == 3);
    assert(tokensSimple[0].es(TipoToken::CorcheteAbre));
    assert(tokensSimple[1].es(TipoToken::Registro) && tokensSimple[1].texto == "rbp");
    assert(tokensSimple[2].es(TipoToken::CorcheteCierra));

    // 4. [rbp-8] sin espacios
    std::vector<Token> tokensSinEspacio = AnalizadorLexico::tokenizarLinea("[rbp-8]", 8);
    assert(tokensSinEspacio.size() == 4);
    assert(tokensSinEspacio[0].es(TipoToken::CorcheteAbre));
    assert(tokensSinEspacio[1].es(TipoToken::Registro) && tokensSinEspacio[1].texto == "rbp");
    assert(tokensSinEspacio[2].es(TipoToken::Inmediato) && tokensSinEspacio[2].texto == "-8");
    assert(tokensSinEspacio[3].es(TipoToken::CorcheteCierra));

    std::cout << "  -> Exito en tokens de memoria.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de AnalizadorLexico ===\n";
    probarTokensIndividuales();
    probarInstruccionTokenizada();
    probarEtiquetasYPrograma();
    probarTokensMemoria();
    std::cout << "Todas las pruebas del analizador lexico pasaron con exito.\n";
    return 0;
}
