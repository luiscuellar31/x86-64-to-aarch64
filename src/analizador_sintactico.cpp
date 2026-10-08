#include "analizador_sintactico.hpp"
#include <cctype>

Operando AnalizadorSintactico::clasificarOperando(const std::string& textoOperando) {
    std::vector<Token> tokens = AnalizadorLexico::tokenizarLinea(textoOperando, 1);
    if (tokens.empty()) {
        return Operando(TipoOperando::Desconocido, "");
    }

    const Token& primerToken = tokens[0];
    if (primerToken.es(TipoToken::Registro)) {
        return Operando::crearRegistro(primerToken.texto);
    }
    if (primerToken.es(TipoToken::Inmediato)) {
        return Operando::crearInmediato(primerToken.texto);
    }
    if (primerToken.es(TipoToken::Identificador)) {
        return Operando::crearEtiqueta(primerToken.texto);
    }
    return Operando(TipoOperando::Desconocido, primerToken.texto);
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

    // 3. Procesar los operandos restantes separados por coma
    while (indice < tokens.size()) {
        const Token& tokenActual = tokens[indice];
        if (tokenActual.es(TipoToken::FinDeLinea)) {
            break;
        }

        if (tokenActual.es(TipoToken::Coma)) {
            indice++;
            continue;
        }

        if (tokenActual.es(TipoToken::Registro)) {
            instruccion.agregarOperando(Operando::crearRegistro(tokenActual.texto));
        } else if (tokenActual.es(TipoToken::Inmediato)) {
            instruccion.agregarOperando(Operando::crearInmediato(tokenActual.texto));
        } else if (tokenActual.es(TipoToken::Identificador)) {
            instruccion.agregarOperando(Operando::crearEtiqueta(tokenActual.texto));
        } else {
            instruccion.agregarOperando(Operando(TipoOperando::Desconocido, tokenActual.texto));
        }
        indice++;
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
