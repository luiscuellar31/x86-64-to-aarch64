#include "analizador_sintactico.hpp"
#include <cctype>

Operando AnalizadorSintactico::clasificarOperando(const std::string& textoOperando) {
    std::vector<Token> tokens = AnalizadorLexico::tokenizarLinea(textoOperando, 1);
    if (tokens.empty()) {
        return Operando(TipoOperando::Desconocido, "");
    }

    const Token& primerToken = tokens[0];
    if (primerToken.es(TipoToken::CorcheteAbre)) {
        size_t indice = 0;
        return analizarOperandoMemoria(tokens, indice);
    }
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

Operando AnalizadorSintactico::analizarOperandoMemoria(const std::vector<Token>& tokens, size_t& indice) {
    if (indice >= tokens.size() || !tokens[indice].es(TipoToken::CorcheteAbre)) {
        return Operando(TipoOperando::Desconocido, "");
    }

    indice++; // Consumir '['
    std::string textoReconstruido = "[";

    if (indice >= tokens.size() || tokens[indice].es(TipoToken::FinDeLinea)) {
        return Operando(TipoOperando::Desconocido, "[");
    }

    // Esperamos un registro base (o identificador)
    if (!tokens[indice].es(TipoToken::Registro) && !tokens[indice].es(TipoToken::Identificador)) {
        while (indice < tokens.size() && !tokens[indice].es(TipoToken::CorcheteCierra) && !tokens[indice].es(TipoToken::FinDeLinea)) {
            textoReconstruido += tokens[indice].texto;
            indice++;
        }
        if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
            textoReconstruido += "]";
            indice++;
        }
        return Operando(TipoOperando::Desconocido, textoReconstruido);
    }

    std::string registroBase = tokens[indice].texto;
    textoReconstruido += registroBase;
    indice++;

    int64_t desplazamiento = 0;

    // Caso 1: Cierre directo [rbp]
    if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
        indice++; // Consumir ']'
        return Operando::crearMemoria(registroBase, 0, 64);
    }

    // Caso 2: Operador Mas o Menos explicito (ej. [rbp - 8], [rbp + 16])
    if (indice < tokens.size() && (tokens[indice].es(TipoToken::Mas) || tokens[indice].es(TipoToken::Menos))) {
        bool esResta = tokens[indice].es(TipoToken::Menos);
        textoReconstruido += esResta ? " - " : " + ";
        indice++;

        if (indice < tokens.size() && tokens[indice].es(TipoToken::Inmediato)) {
            int64_t valorNum = 0;
            try {
                valorNum = std::stoll(tokens[indice].texto, nullptr, 0);
            } catch (...) {
                valorNum = 0;
            }
            desplazamiento = esResta ? -valorNum : valorNum;
            textoReconstruido += tokens[indice].texto;
            indice++;
        } else {
            // Sintaxis invalida dentro del corchete
            while (indice < tokens.size() && !tokens[indice].es(TipoToken::CorcheteCierra) && !tokens[indice].es(TipoToken::FinDeLinea)) {
                textoReconstruido += tokens[indice].texto;
                indice++;
            }
            if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
                textoReconstruido += "]";
                indice++;
            }
            return Operando(TipoOperando::Desconocido, textoReconstruido);
        }
    }
    // Caso 3: Inmediato con signo ya incorporado (ej. [rbp-8] donde el lexer agrupo "-8")
    else if (indice < tokens.size() && tokens[indice].es(TipoToken::Inmediato)) {
        int64_t valorNum = 0;
        try {
            valorNum = std::stoll(tokens[indice].texto, nullptr, 0);
        } catch (...) {
            valorNum = 0;
        }
        desplazamiento = valorNum;
        if (desplazamiento >= 0) {
            textoReconstruido += " + " + std::to_string(desplazamiento);
        } else {
            textoReconstruido += " - " + std::to_string(-desplazamiento);
        }
        indice++;
    }

    // El token siguiente debe ser CorcheteCierra ']'
    if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
        indice++; // Consumir ']'
        return Operando::crearMemoria(registroBase, desplazamiento, 64);
    }

    // Si hay tokens adicionales no soportados (ej. indexacion compleja [base+index*4])
    while (indice < tokens.size() && !tokens[indice].es(TipoToken::CorcheteCierra) && !tokens[indice].es(TipoToken::FinDeLinea)) {
        textoReconstruido += " " + tokens[indice].texto;
        indice++;
    }
    if (indice < tokens.size() && tokens[indice].es(TipoToken::CorcheteCierra)) {
        textoReconstruido += "]";
        indice++;
    }

    return Operando(TipoOperando::Desconocido, textoReconstruido);
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

        if (tokenActual.es(TipoToken::CorcheteAbre)) {
            instruccion.agregarOperando(analizarOperandoMemoria(tokens, indice));
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
