#ifndef OPCIONES_CLI_HPP
#define OPCIONES_CLI_HPP

#include <string>
#include <iostream>
#include "resultado_traduccion.hpp"

/**
 * @brief Estructura que almacena los parámetros y banderas configurados en la línea de comandos.
 */
struct OpcionesCLI {
    std::string archivoEntrada;
    std::string archivoSalida;
    bool mostrarAyuda = false;
    bool mostrarVersion = false;
    bool explicar = false;
    bool volcarIR = false;
    bool ejecutarDemo = false;
};

/**
 * @brief Resultado del análisis sintáctico de los argumentos de línea de comandos.
 */
struct ResultadoParseoCLI {
    bool exito = true;
    std::string mensajeError;
    OpcionesCLI opciones;
};

/**
 * @brief Analizador y despachador de opciones de línea de comandos.
 */
class AnalizadorCLI {
public:
    /**
     * @brief Analiza los argumentos pasados en argc y argv.
     * @param argc Cantidad de argumentos.
     * @param argv Vector de cadenas de argumentos.
     * @return Estructura con el resultado del parseo y opciones configuradas.
     */
    static ResultadoParseoCLI parsear(int argc, char* argv[]);

    /**
     * @brief Muestra el texto de ayuda y uso del CLI en el flujo especificado.
     */
    static void mostrarAyuda(std::ostream& salida = std::cout);

    /**
     * @brief Muestra la versión del traductor en el flujo especificado.
     */
    static void mostrarVersion(std::ostream& salida = std::cout);

    /**
     * @brief Ejecuta el flujo principal del CLI de acuerdo con las opciones proporcionadas.
     * @param opciones Configuración de ejecución.
     * @param salida Flujo para salidas estándar o informativas.
     * @param error Flujo para mensajes de error o diagnósticos.
     * @return Código de salida del proceso (0 para éxito, distinto de 0 en error).
     */
    static int ejecutar(const OpcionesCLI& opciones, std::ostream& salida = std::cout, std::ostream& error = std::cerr);
};

#endif // OPCIONES_CLI_HPP
