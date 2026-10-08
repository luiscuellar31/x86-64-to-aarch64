#ifndef NORMALIZADOR_HPP
#define NORMALIZADOR_HPP

#include <string>
#include <vector>

/**
 * @brief Estructura que almacena una linea normalizada y su numero de linea de origen.
 */
struct LineaNormalizada {
    int numeroLineaOriginal;
    std::string contenido;

    LineaNormalizada(int lineaOriginal, const std::string& textoNormalizado);
};

/**
 * @brief Modulo para preprocesamiento y limpieza de codigo fuente x86-64.
 *
 * Responsabilidades:
 * - Eliminar comentarios (delimitados por ';').
 * - Recortar espacios y tabulaciones superfluos.
 * - Ignorar lineas vacias conservando la numeracion de origen.
 * - Estandarizar mnemonicos y nombres de registros a minusculas.
 * - Conservar el nombre exacto de las etiquetas (case sensitive).
 */
class Normalizador {
public:
    /**
     * @brief Limpia y normaliza una sola linea de texto.
     * @param lineaCruda Texto original de la linea.
     * @return Linea limpia y estandarizada, o cadena vacia si no contiene instrucciones.
     */
    static std::string normalizarLinea(const std::string& lineaCruda);

    /**
     * @brief Normaliza un archivo o bloque completo de codigo fuente.
     * @param codigoFuente Texto completo en ensamblador x86-64.
     * @return Vector de lineas utiles con su numero de linea original.
     */
    static std::vector<LineaNormalizada> normalizarCodigo(const std::string& codigoFuente);
};

#endif // NORMALIZADOR_HPP
