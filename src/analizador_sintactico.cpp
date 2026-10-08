#include "analizador_sintactico.hpp"
#include "tabla_registros.hpp"
#include <sstream>
#include <cctype>

static std::string recortar(const std::string& texto) {
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

static bool esIdentificadorValido(const std::string& texto) {
    if (texto.empty()) {
        return false;
    }
    char primerCaracter = texto[0];
    if (primerCaracter != '_' && (std::isalpha(static_cast<unsigned char>(primerCaracter)) == 0)) {
        return false;
    }
    for (char caracter : texto) {
        if (caracter != '_' && (std::isalnum(static_cast<unsigned char>(caracter)) == 0)) {
            return false;
        }
    }
    return true;
}

static bool esInmediato(const std::string& texto) {
    if (texto.empty()) {
        return false;
    }
    size_t indice = 0;
    if (texto[0] == '+' || texto[0] == '-') {
        indice++;
        if (indice >= texto.size()) {
            return false;
        }
    }
    if (texto.substr(indice, 2) == "0x" || texto.substr(indice, 2) == "0X") {
        indice += 2;
        if (indice >= texto.size()) {
            return false;
        }
        for (size_t i = indice; i < texto.size(); ++i) {
            if (std::isxdigit(static_cast<unsigned char>(texto[i])) == 0) {
                return false;
            }
        }
        return true;
    }
    for (size_t i = indice; i < texto.size(); ++i) {
        if (std::isdigit(static_cast<unsigned char>(texto[i])) == 0) {
            return false;
        }
    }
    return true;
}

Operando AnalizadorSintactico::clasificarOperando(const std::string& textoOperando) {
    std::string limpio = recortar(textoOperando);
    if (limpio.empty()) {
        return Operando(TipoOperando::Desconocido, "");
    }

    if (esRegistroX86(limpio)) {
        return Operando::crearRegistro(normalizarNombreRegistro(limpio));
    }

    if (esInmediato(limpio)) {
        return Operando::crearInmediato(limpio);
    }

    if (esIdentificadorValido(limpio)) {
        return Operando::crearEtiqueta(limpio);
    }

    return Operando(TipoOperando::Desconocido, limpio);
}

Instruccion AnalizadorSintactico::analizarLinea(const LineaNormalizada& linea) {
    std::string contenido = recortar(linea.contenido);
    if (contenido.empty()) {
        return Instruccion(CodigoOperacion::Desconocido, linea.numeroLineaOriginal);
    }

    std::string etiquetaDetectada = "";
    std::string textoCuerpo = contenido;

    // Detectamos si la linea inicia con definicion de etiqueta (delimitada por ':')
    size_t posicionDosPuntos = contenido.find(':');
    if (posicionDosPuntos != std::string::npos) {
        etiquetaDetectada = recortar(contenido.substr(0, posicionDosPuntos));
        textoCuerpo = recortar(contenido.substr(posicionDosPuntos + 1));

        // Caso 1: Es una etiqueta pura sin instruccion acompañante (ej. "inicio:")
        if (textoCuerpo.empty()) {
            Instruccion instruccionEtiqueta(CodigoOperacion::Etiqueta, linea.numeroLineaOriginal);
            instruccionEtiqueta.etiqueta = etiquetaDetectada;
            return instruccionEtiqueta;
        }
    }

    // Caso 2: Procesamos el cuerpo de la instruccion (mnemonico y operandos)
    size_t posicionPrimerEspacio = 0;
    while (posicionPrimerEspacio < textoCuerpo.size() &&
           (std::isspace(static_cast<unsigned char>(textoCuerpo[posicionPrimerEspacio])) == 0)) {
        posicionPrimerEspacio++;
    }

    std::string textoMnemonico = textoCuerpo.substr(0, posicionPrimerEspacio);
    CodigoOperacion codigo = textoACodigoOperacion(textoMnemonico);

    Instruccion instruccion(codigo, linea.numeroLineaOriginal);
    if (!etiquetaDetectada.empty()) {
        instruccion.etiqueta = etiquetaDetectada;
    }

    // Si no hay operandos (ej. "ret")
    if (posicionPrimerEspacio >= textoCuerpo.size()) {
        return instruccion;
    }

    std::string textoOperandos = recortar(textoCuerpo.substr(posicionPrimerEspacio));
    if (textoOperandos.empty()) {
        return instruccion;
    }

    // Dividimos operandos separados por coma
    std::stringstream flujo(textoOperandos);
    std::string item;
    while (std::getline(flujo, item, ',')) {
        std::string operandoLimpio = recortar(item);
        if (!operandoLimpio.empty()) {
            instruccion.agregarOperando(clasificarOperando(operandoLimpio));
        }
    }

    return instruccion;
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
