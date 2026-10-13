#include "analizador_sintactico.hpp"
#include <cctype>

static Operando clasificarTokensOperando(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        return Operando(TipoOperando::Desconocido, "");
    }

    const Token& primerToken = tokens[0];
    if (primerToken.es(TipoToken::CorcheteAbre)) {
        size_t indice = 0;
        Operando operando = AnalizadorSintactico::analizarOperandoMemoria(tokens, indice);
        if (indice == tokens.size()) {
            return operando;
        }
    } else if (tokens.size() == 1) {
        if (primerToken.es(TipoToken::Registro)) {
            return Operando::crearRegistro(primerToken.texto);
        }
        if (primerToken.es(TipoToken::Inmediato)) {
            return Operando::crearInmediato(primerToken.texto);
        }
        if (primerToken.es(TipoToken::Identificador)) {
            return Operando::crearEtiqueta(primerToken.texto);
        }
    }
    return Operando(TipoOperando::Desconocido, primerToken.texto);
}

Operando AnalizadorSintactico::clasificarOperando(const std::string& textoOperando) {
    return clasificarTokensOperando(AnalizadorLexico::tokenizarLinea(textoOperando, 1));
}

Operando AnalizadorSintactico::analizarOperandoMemoria(const std::vector<Token>& tokens, size_t& indice) {
    if (indice >= tokens.size() || !tokens[indice].es(TipoToken::CorcheteAbre)) {
        return Operando(TipoOperando::Desconocido, "");
    }

    size_t inicio = indice;
    indice++;
    while (indice < tokens.size() && !tokens[indice].es(TipoToken::CorcheteCierra) &&
           !tokens[indice].es(TipoToken::FinDeLinea)) {
        indice++;
    }
    if (indice >= tokens.size() || !tokens[indice].es(TipoToken::CorcheteCierra)) {
        return Operando(TipoOperando::Desconocido, "[");
    }
    size_t cierre = indice;
    indice++; // Consumir ']'; las formas invalidas tambien avanzan el indice.

    if (cierre < inicio + 2 ||
        (!tokens[inicio + 1].es(TipoToken::Registro) && !tokens[inicio + 1].es(TipoToken::Identificador))) {
        return Operando(TipoOperando::Desconocido, "[");
    }
    std::string registroBase = tokens[inicio + 1].texto;
    if (cierre == inicio + 2) {
        return Operando::crearMemoria(registroBase, 0, 64);
    }

    std::string literal;
    const Token& tokenDesplazamiento = tokens[inicio + 2];
    if (cierre == inicio + 3 && tokenDesplazamiento.es(TipoToken::Inmediato) &&
        !tokenDesplazamiento.texto.empty() &&
        (tokenDesplazamiento.texto[0] == '+' || tokenDesplazamiento.texto[0] == '-')) {
        literal = tokenDesplazamiento.texto;
    } else if (cierre == inicio + 4 &&
               (tokenDesplazamiento.es(TipoToken::Mas) || tokenDesplazamiento.es(TipoToken::Menos))) {
        const Token& magnitud = tokens[inicio + 3];
        if (magnitud.es(TipoToken::Inmediato) && !magnitud.texto.empty() &&
            magnitud.texto[0] != '+' && magnitud.texto[0] != '-') {
            literal = tokenDesplazamiento.texto + magnitud.texto;
        }
    }

    // Convertir con el signo incluido evita negar INT64_MIN y detecta desbordamientos.
    Operando desplazamiento = Operando::crearInmediato(literal);
    if (!desplazamiento.esInmediato()) {
        return Operando(TipoOperando::Desconocido, "[");
    }
    return Operando::crearMemoria(registroBase, desplazamiento.valorNumerico, 64);
}

Instruccion AnalizadorSintactico::analizarTokens(const std::vector<Token>& tokens) {
    if (tokens.empty()) {
        return Instruccion(CodigoOperacion::Desconocido, 1);
    }

    int numeroLinea = tokens[0].numeroLinea;
    size_t indice = 0;
    std::string etiquetaDetectada = "";

    // 1. Detección de etiqueta al inicio (Identificador seguido de ':')
    if (tokens.size() >= 2 && tokens[0].es(TipoToken::Identificador) && tokens[1].es(TipoToken::DosPuntos)) {
        etiquetaDetectada = tokens[0].texto;
        indice = 2; // Avanzamos tras el identificador y los dos puntos

        // Si la línea solo contenía la etiqueta
        if (indice >= tokens.size() || tokens[indice].es(TipoToken::FinDeLinea)) {
            Instruccion instruccionEtiqueta(CodigoOperacion::Etiqueta, numeroLinea);
            instruccionEtiqueta.etiqueta = etiquetaDetectada;
            return instruccionEtiqueta;
        }
    }

    // 2. Extraer el mnemónico
    if (indice >= tokens.size() || tokens[indice].es(TipoToken::FinDeLinea)) {
        Instruccion instVacia(CodigoOperacion::Desconocido, numeroLinea);
        instVacia.etiqueta = etiquetaDetectada;
        return instVacia;
    }

    if (!tokens[indice].es(TipoToken::Identificador)) {
        Instruccion invalida(CodigoOperacion::Desconocido, numeroLinea);
        invalida.errorSintactico = Diagnostico::crearError(
            CodigosDiagnostico::kSintaxisInvalida, numeroLinea, "se esperaba un mnemonico o una etiqueta valida");
        return invalida;
    }

    std::string mnemonicoTexto = tokens[indice].texto;
    std::string mnemonicoNormalizado = "";
    for (char c : mnemonicoTexto) {
        mnemonicoNormalizado += static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    CodigoOperacion codigo = textoACodigoOperacion(mnemonicoNormalizado);

    Instruccion instruccion(codigo, numeroLinea);
    if (!etiquetaDetectada.empty()) {
        instruccion.etiqueta = etiquetaDetectada;
    }
    indice++;

    if (indice < tokens.size() && !tokens[indice].es(TipoToken::FinDeLinea) &&
        tokens[indice].columna <= tokens[indice - 1].columna + static_cast<int>(mnemonicoTexto.size())) {
        instruccion.errorSintactico = Diagnostico::crearError(
            CodigosDiagnostico::kSintaxisInvalida, numeroLinea, "el mnemonico y sus operandos requieren un espacio separador");
        return instruccion;
    }

    // Cada campo entre comas debe contener exactamente un operando completo.
    while (indice < tokens.size() && !tokens[indice].es(TipoToken::FinDeLinea)) {
        size_t inicioOperando = indice;
        int profundidadCorchetes = 0;
        while (indice < tokens.size() && !tokens[indice].es(TipoToken::FinDeLinea)) {
            if (tokens[indice].es(TipoToken::Coma) && profundidadCorchetes == 0) {
                break;
            }
            if (tokens[indice].es(TipoToken::CorcheteAbre)) {
                profundidadCorchetes++;
            } else if (tokens[indice].es(TipoToken::CorcheteCierra)) {
                profundidadCorchetes--;
            }
            indice++;
        }

        std::vector<Token> tokensOperando(tokens.begin() + inicioOperando, tokens.begin() + indice);
        Operando operando = clasificarTokensOperando(tokensOperando);
        instruccion.agregarOperando(operando);
        if (operando.tipo == TipoOperando::Desconocido) {
            bool esLiteralInvalido = tokensOperando.size() == 1 && tokensOperando[0].es(TipoToken::Inmediato);
            std::string codigoError = esLiteralInvalido ? CodigosDiagnostico::kInmediatoInvalido
                                                     : CodigosDiagnostico::kSintaxisInvalida;
            std::string mensaje = esLiteralInvalido ? "literal inmediato invalido o fuera del rango de int64_t"
                                                   : "se esperaba un operando completo; revise comas, corchetes y signos";
            instruccion.errorSintactico = Diagnostico::crearError(codigoError, numeroLinea, mensaje);
            return instruccion;
        }

        if (indice < tokens.size() && tokens[indice].es(TipoToken::Coma)) {
            indice++;
            if (indice >= tokens.size() || tokens[indice].es(TipoToken::FinDeLinea)) {
                instruccion.errorSintactico = Diagnostico::crearError(
                    CodigosDiagnostico::kSintaxisInvalida, numeroLinea, "falta un operando despues de la coma");
                return instruccion;
            }
        }
    }

    return instruccion;
}

Instruccion AnalizadorSintactico::analizarLinea(const LineaNormalizada& linea) {
    std::vector<Token> tokens = AnalizadorLexico::tokenizarLinea(linea.contenido, linea.numeroLineaOriginal);
    return analizarTokens(tokens);
}

Instruccion AnalizadorSintactico::analizarTexto(const std::string& textoLinea, int numeroLinea) {
    std::string normalizado = Normalizador::normalizarLinea(textoLinea);
    if (normalizado.empty()) {
        return Instruccion(CodigoOperacion::Desconocido, numeroLinea);
    }
    return analizarLinea(LineaNormalizada(numeroLinea, normalizado));
}

std::vector<Instruccion> AnalizadorSintactico::analizarPrograma(const std::string& codigoFuente) {
    std::vector<Instruccion> instrucciones;
    std::vector<LineaNormalizada> lineasNormalizadas = Normalizador::normalizarCodigo(codigoFuente);

    for (const auto& linea : lineasNormalizadas) {
        instrucciones.push_back(analizarLinea(linea));
    }

    return instrucciones;
}
