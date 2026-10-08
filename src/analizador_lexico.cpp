#include "analizador_lexico.hpp"
#include "tabla_registros.hpp"
#include <cctype>
#include <sstream>

std::string tipoTokenATexto(TipoToken tipo) {
    switch (tipo) {
        case TipoToken::Identificador:
            return "Identificador";
        case TipoToken::Registro:
            return "Registro";
        case TipoToken::Inmediato:
            return "Inmediato";
        case TipoToken::Coma:
            return "Coma";
        case TipoToken::DosPuntos:
            return "DosPuntos";
        case TipoToken::FinDeLinea:
            return "FinDeLinea";
        case TipoToken::Desconocido:
        default:
            return "Desconocido";
    }
}

Token::Token()
    : tipo(TipoToken::Desconocido), texto(""), numeroLinea(1), columna(1) {
}

Token::Token(TipoToken tipoToken, const std::string& valorTexto, int linea, int col)
    : tipo(tipoToken), texto(valorTexto), numeroLinea(linea), columna(col) {
}

bool Token::es(TipoToken tipoEsperado) const {
    return tipo == tipoEsperado;
}

std::string Token::aTexto() const {
    return "[" + tipoTokenATexto(tipo) + ": \"" + texto + "\" (L" +
           std::to_string(numeroLinea) + ", C" + std::to_string(columna) + ")]";
}

std::vector<Token> AnalizadorLexico::tokenizarLinea(const std::string& textoLinea, int numeroLinea) {
    std::vector<Token> tokens;
    size_t longitud = textoLinea.size();
    size_t indice = 0;

    while (indice < longitud) {
        char caracterActual = textoLinea[indice];

        // 1. Ignorar espacios en blanco
        if (std::isspace(static_cast<unsigned char>(caracterActual)) != 0) {
            indice++;
            continue;
        }

        // 2. Si encontramos un comentario ';', el resto de la linea se ignora
        if (caracterActual == ';') {
            break;
        }

        int columnaActual = static_cast<int>(indice) + 1;

        // 3. Coma ','
        if (caracterActual == ',') {
            tokens.push_back(Token(TipoToken::Coma, ",", numeroLinea, columnaActual));
            indice++;
            continue;
        }

        // 4. Dos puntos ':'
        if (caracterActual == ':') {
            tokens.push_back(Token(TipoToken::DosPuntos, ":", numeroLinea, columnaActual));
            indice++;
            continue;
        }

        // 5. Constantes inmediatas (numeros con o sin signo, decimales o hexadecimales)
        bool esSignoConDigito = (caracterActual == '+' || caracterActual == '-') &&
                                (indice + 1 < longitud) &&
                                (std::isdigit(static_cast<unsigned char>(textoLinea[indice + 1])) != 0);

        bool esDigito = (std::isdigit(static_cast<unsigned char>(caracterActual)) != 0);

        if (esSignoConDigito || esDigito) {
            size_t inicioNum = indice;

            // Consumir signo si existe
            if (caracterActual == '+' || caracterActual == '-') {
                indice++;
            }

            // Comprobar si es hexadecimal con prefijo 0x o 0X
            if (indice + 1 < longitud && textoLinea[indice] == '0' &&
                (textoLinea[indice + 1] == 'x' || textoLinea[indice + 1] == 'X')) {
                indice += 2;
                while (indice < longitud && (std::isxdigit(static_cast<unsigned char>(textoLinea[indice])) != 0)) {
                    indice++;
                }
            } else {
                // Decimal regular
                while (indice < longitud && (std::isdigit(static_cast<unsigned char>(textoLinea[indice])) != 0)) {
                    indice++;
                }
            }

            std::string literalInmediato = textoLinea.substr(inicioNum, indice - inicioNum);
            tokens.push_back(Token(TipoToken::Inmediato, literalInmediato, numeroLinea, columnaActual));
            continue;
        }

        // 6. Identificadores (mnemonicos, registros o etiquetas)
        if (std::isalpha(static_cast<unsigned char>(caracterActual)) != 0 || caracterActual == '_') {
            size_t inicioIdentificador = indice;
            while (indice < longitud &&
                   (std::isalnum(static_cast<unsigned char>(textoLinea[indice])) != 0 || textoLinea[indice] == '_')) {
                indice++;
            }

            std::string palabra = textoLinea.substr(inicioIdentificador, indice - inicioIdentificador);

            // Verificamos si la palabra es un registro conocido de x86-64 (o subregistro)
            if (esRegistroX86Cualquiera(palabra)) {
                tokens.push_back(Token(TipoToken::Registro, normalizarNombreRegistro(palabra), numeroLinea, columnaActual));
            } else {
                tokens.push_back(Token(TipoToken::Identificador, palabra, numeroLinea, columnaActual));
            }
            continue;
        }

        // 7. Caracter no reconocido
        std::string caracterDesconocido(1, caracterActual);
        tokens.push_back(Token(TipoToken::Desconocido, caracterDesconocido, numeroLinea, columnaActual));
        indice++;
    }

    return tokens;
}

std::vector<Token> AnalizadorLexico::tokenizarPrograma(const std::string& codigoFuente) {
    std::vector<Token> todosLosTokens;
    std::stringstream flujo(codigoFuente);
    std::string linea;
    int numeroLinea = 1;

    while (std::getline(flujo, linea)) {
        std::vector<Token> tokensDeLinea = tokenizarLinea(linea, numeroLinea);
        if (!tokensDeLinea.empty()) {
            for (const auto& token : tokensDeLinea) {
                todosLosTokens.push_back(token);
            }
            todosLosTokens.push_back(Token(TipoToken::FinDeLinea, "\n", numeroLinea, static_cast<int>(linea.size()) + 1));
        }
        numeroLinea++;
    }

    return todosLosTokens;
}
