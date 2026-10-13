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

static Operando rechazarMemoriaInvalida(const std::vector<Token>& tokens, size_t& indice) {
    while (indice < tokens.size() && !tokens[indice].es(TipoToken::CorcheteCierra) &&
           !tokens[indice].es(TipoToken::FinDeLinea)) {
        indice++;
    }
    if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
        indice++;
    }
    return Operando(TipoOperando::Desconocido, "[");
}

Operando AnalizadorSintactico::analizarOperandoMemoria(const std::vector<Token>& tokens, size_t& indice) {
    if (indice >= tokens.size() || !tokens[indice].es(TipoToken::CorcheteAbre)) {
        return Operando(TipoOperando::Desconocido, "");
    }
    indice++;

    if (indice >= tokens.size()) {
        return rechazarMemoriaInvalida(tokens, indice);
    }
    const Token& base = tokens[indice];
    if (!base.es(TipoToken::Registro) && !base.es(TipoToken::Identificador)) {
        return rechazarMemoriaInvalida(tokens, indice);
    }
    std::string registroBase = base.texto;
    indice++;

    if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
        indice++;
        return Operando::crearMemoria(registroBase, 0, 64);
    }
    if (indice >= tokens.size()) {
        return rechazarMemoriaInvalida(tokens, indice);
    }

    std::string literalDesplazamiento;
    const Token& tokenDesplazamiento = tokens[indice];
    if (tokenDesplazamiento.es(TipoToken::Mas) || tokenDesplazamiento.es(TipoToken::Menos)) {
        std::string signo = tokenDesplazamiento.texto;
        indice++;
        if (indice >= tokens.size() || !tokens[indice].es(TipoToken::Inmediato)) {
            return rechazarMemoriaInvalida(tokens, indice);
        }
        const std::string& magnitud = tokens[indice].texto;
        if (magnitud.empty() || magnitud[0] == '+' || magnitud[0] == '-') {
            return rechazarMemoriaInvalida(tokens, indice);
        }
        literalDesplazamiento = signo + magnitud;
        indice++;
    } else if (tokenDesplazamiento.es(TipoToken::Inmediato)) {
        const std::string& literal = tokenDesplazamiento.texto;
        if (literal.empty() || (literal[0] != '+' && literal[0] != '-')) {
            return rechazarMemoriaInvalida(tokens, indice);
        }
        literalDesplazamiento = literal;
        indice++;
    } else {
        return rechazarMemoriaInvalida(tokens, indice);
    }

    if (indice >= tokens.size() || !tokens[indice].es(TipoToken::CorcheteCierra)) {
        return rechazarMemoriaInvalida(tokens, indice);
    }
    indice++;

    // Convertir con el signo incluido evita negar INT64_MIN y detecta desbordamientos.
    Operando desplazamiento = Operando::crearInmediato(literalDesplazamiento);
    if (!desplazamiento.esInmediato()) {
        return Operando(TipoOperando::Desconocido, "[");
    }
    return Operando::crearMemoria(registroBase, desplazamiento.valorNumerico, 64);
}

static std::vector<Token> leerTokensOperando(const std::vector<Token>& tokens, size_t& indice) {
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
    return std::vector<Token>(tokens.begin() + inicioOperando, tokens.begin() + indice);
}

static Diagnostico crearErrorOperando(const std::vector<Token>& tokensOperando, int numeroLinea) {
    bool esLiteralInvalido = tokensOperando.size() == 1 && tokensOperando[0].es(TipoToken::Inmediato);
    if (esLiteralInvalido) {
        return Diagnostico::crearError(CodigosDiagnostico::kInmediatoInvalido, numeroLinea,
                                      "literal inmediato invalido o fuera del rango de int64_t");
    }
    return Diagnostico::crearError(CodigosDiagnostico::kSintaxisInvalida, numeroLinea,
                                  "se esperaba un operando completo; revise comas, corchetes y signos");
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
        Instruccion instruccionVacia(CodigoOperacion::Desconocido, numeroLinea);
        instruccionVacia.etiqueta = etiquetaDetectada;
        return instruccionVacia;
    }

    if (!tokens[indice].es(TipoToken::Identificador)) {
        Instruccion invalida(CodigoOperacion::Desconocido, numeroLinea);
        invalida.errorSintactico = Diagnostico::crearError(
            CodigosDiagnostico::kSintaxisInvalida, numeroLinea, "se esperaba un mnemonico o una etiqueta valida");
        return invalida;
    }

    std::string mnemonicoTexto = tokens[indice].texto;
    std::string mnemonicoNormalizado = "";
    for (char caracter : mnemonicoTexto) {
        mnemonicoNormalizado += static_cast<char>(std::tolower(static_cast<unsigned char>(caracter)));
    }
    CodigoOperacion codigoOperacion = textoACodigoOperacion(mnemonicoNormalizado);
    int columnaFinMnemonico = tokens[indice].columna + static_cast<int>(mnemonicoTexto.size());

    Instruccion instruccion(codigoOperacion, numeroLinea);
    if (!etiquetaDetectada.empty()) {
        instruccion.etiqueta = etiquetaDetectada;
    }
    indice++;

    if (indice < tokens.size() && !tokens[indice].es(TipoToken::FinDeLinea) &&
        tokens[indice].columna <= columnaFinMnemonico) {
        instruccion.errorSintactico = Diagnostico::crearError(
            CodigosDiagnostico::kSintaxisInvalida, numeroLinea, "el mnemonico y sus operandos requieren un espacio separador");
        return instruccion;
    }

    // Cada campo entre comas debe contener exactamente un operando completo.
    while (indice < tokens.size() && !tokens[indice].es(TipoToken::FinDeLinea)) {
        std::vector<Token> tokensOperando = leerTokensOperando(tokens, indice);
        Operando operando = clasificarTokensOperando(tokensOperando);
        instruccion.agregarOperando(operando);
        if (operando.tipo == TipoOperando::Desconocido) {
            instruccion.errorSintactico = crearErrorOperando(tokensOperando, numeroLinea);
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
