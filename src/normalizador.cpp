#include "normalizador.hpp"
#include "tabla_registros.hpp"
#include <sstream>
#include <cctype>

LineaNormalizada::LineaNormalizada(int lineaOriginal, const std::string& textoNormalizado)
    : numeroLineaOriginal(lineaOriginal), contenido(textoNormalizado) {
}

static std::string recortarEspacios(const std::string& texto) {
    if (texto.empty()) {
        return "";
    }

    size_t inicio = 0;
    while (inicio < texto.size() && (std::isspace(static_cast<unsigned char>(texto[inicio])) != 0)) {
        inicio++;
    }

    if (inicio >= texto.size()) {
        return "";
    }

    size_t fin = texto.size() - 1;
    while (fin > inicio && (std::isspace(static_cast<unsigned char>(texto[fin])) != 0)) {
        fin--;
    }

    return texto.substr(inicio, fin - inicio + 1);
}

static std::string aMinusculas(const std::string& texto) {
    std::string resultado = "";
    for (char caracter : texto) {
        resultado += static_cast<char>(std::tolower(static_cast<unsigned char>(caracter)));
    }
    return resultado;
}

static bool esInmediatoNumerico(const std::string& texto) {
    if (texto.empty()) {
        return false;
    }
    char primerCaracter = texto[0];
    if (primerCaracter == '+' || primerCaracter == '-' || std::isdigit(static_cast<unsigned char>(primerCaracter)) != 0) {
        return true;
    }
    if (texto.size() >= 2 && (texto.substr(0, 2) == "0x" || texto.substr(0, 2) == "0X")) {
        return true;
    }
    return false;
}

// Procesa la parte correspondiente a la instruccion (mnemonico y operandos)
static std::string normalizarInstruccion(const std::string& textoInstruccion) {
    std::string limpia = recortarEspacios(textoInstruccion);
    if (limpia.empty()) {
        return "";
    }

    // Separamos el primer token (mnemonico) del resto de operandos
    size_t posicionEspacio = 0;
    while (posicionEspacio < limpia.size() && (std::isspace(static_cast<unsigned char>(limpia[posicionEspacio])) == 0)) {
        posicionEspacio++;
    }

    std::string mnemonicoCrudo = limpia.substr(0, posicionEspacio);
    std::string mnemonico = aMinusculas(mnemonicoCrudo);

    if (posicionEspacio >= limpia.size()) {
        // Instruccion sin operandos (ej. "ret")
        return mnemonico;
    }

    std::string restoOperandos = recortarEspacios(limpia.substr(posicionEspacio));
    if (restoOperandos.empty()) {
        return mnemonico;
    }

    // Dividimos los operandos por coma ','
    std::vector<std::string> listaOperandos;
    std::stringstream flujoOperandos(restoOperandos);
    std::string operandoItem;

    while (std::getline(flujoOperandos, operandoItem, ',')) {
        std::string operandoRecortado = recortarEspacios(operandoItem);
        if (!operandoRecortado.empty()) {
            if (esRegistroX86(operandoRecortado)) {
                // Los registros se normalizan a minusculas
                listaOperandos.push_back(normalizarNombreRegistro(operandoRecortado));
            } else if (esInmediatoNumerico(operandoRecortado)) {
                // Los inmediatos numericos se pasan a minusculas (ej. 0x1F -> 0x1f)
                listaOperandos.push_back(aMinusculas(operandoRecortado));
            } else {
                // Identificadores de etiquetas conservan su case exacto
                listaOperandos.push_back(operandoRecortado);
            }
        }
    }

    // Ensamblamos la instruccion con formato estandar: mnemonico op1, op2
    std::string resultado = mnemonico;
    if (!listaOperandos.empty()) {
        resultado += " ";
        for (size_t indice = 0; indice < listaOperandos.size(); ++indice) {
            if (indice > 0) {
                resultado += ", ";
            }
            resultado += listaOperandos[indice];
        }
    }

    return resultado;
}

std::string Normalizador::normalizarLinea(const std::string& lineaCruda) {
    // Paso 1: Eliminar comentarios posteriores a ';'
    size_t posicionComentario = lineaCruda.find(';');
    std::string sinComentarios = (posicionComentario != std::string::npos)
                                     ? lineaCruda.substr(0, posicionComentario)
                                     : lineaCruda;

    // Paso 2: Recortar espacios en blanco
    std::string lineaLimpia = recortarEspacios(sinComentarios);
    if (lineaLimpia.empty()) {
        return "";
    }

    // Paso 3: Identificar si existe etiqueta (delimitada por ':')
    size_t posicionDosPuntos = lineaLimpia.find(':');
    if (posicionDosPuntos != std::string::npos) {
        std::string etiqueta = recortarEspacios(lineaLimpia.substr(0, posicionDosPuntos)) + ":";
        std::string resto = recortarEspacios(lineaLimpia.substr(posicionDosPuntos + 1));

        if (resto.empty()) {
            return etiqueta;
        }

        std::string instruccionNormalizada = normalizarInstruccion(resto);
        if (instruccionNormalizada.empty()) {
            return etiqueta;
        }

        return etiqueta + " " + instruccionNormalizada;
    }

    // Si no contiene etiqueta, normalizamos la instruccion directamente
    return normalizarInstruccion(lineaLimpia);
}

std::vector<LineaNormalizada> Normalizador::normalizarCodigo(const std::string& codigoFuente) {
    std::vector<LineaNormalizada> lineasResultado;
    std::stringstream flujoLineas(codigoFuente);
    std::string lineaActual;
    int numeroLinea = 1;

    while (std::getline(flujoLineas, lineaActual)) {
        std::string lineaNormalizada = normalizarLinea(lineaActual);
        if (!lineaNormalizada.empty()) {
            lineasResultado.push_back(LineaNormalizada(numeroLinea, lineaNormalizada));
        }
        numeroLinea++;
    }

    return lineasResultado;
}
