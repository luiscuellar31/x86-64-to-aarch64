#include "opciones_cli.hpp"
#include "traductor.hpp"
#include <fstream>
#include <sstream>

ResultadoParseoCLI AnalizadorCLI::parsear(int argc, char* argv[]) {
    ResultadoParseoCLI resultado;

    // Sin argumentos se ejecuta la demostracion interactiva por defecto
    if (argc <= 1) {
        resultado.opciones.ejecutarDemo = true;
        return resultado;
    }

    for (int indiceArgumento = 1; indiceArgumento < argc; ++indiceArgumento) {
        std::string argumento = argv[indiceArgumento];

        if (argumento == "-h" || argumento == "--help") {
            resultado.opciones.mostrarAyuda = true;
        } else if (argumento == "-v" || argumento == "--version") {
            resultado.opciones.mostrarVersion = true;
        } else if (argumento == "--explain") {
            resultado.opciones.explicar = true;
        } else if (argumento == "--dump-ir") {
            resultado.opciones.volcarIR = true;
        } else if (argumento == "-o" || argumento == "--output") {
            if (indiceArgumento + 1 < argc) {
                indiceArgumento++;
                resultado.opciones.archivoSalida = argv[indiceArgumento];
            } else {
                resultado.exito = false;
                resultado.mensajeError = "Error: la opción '" + argumento + "' requiere un argumento con la ruta del archivo.";
                return resultado;
            }
        } else if (argumento.rfind("-o", 0) == 0 && argumento.size() > 2) {
            // Soporte para forma pegada: -oarchivo.s
            resultado.opciones.archivoSalida = argumento.substr(2);
        } else if (!argumento.empty() && argumento[0] == '-') {
            resultado.exito = false;
            resultado.mensajeError = "Error: opción desconocida '" + argumento + "'. Use --help para consultar las opciones disponibles.";
            return resultado;
        } else {
            // Argumento posicional (archivo de entrada)
            if (resultado.opciones.archivoEntrada.empty()) {
                resultado.opciones.archivoEntrada = argumento;
            } else {
                resultado.exito = false;
                resultado.mensajeError = "Error: se especificó más de un archivo de entrada ('" +
                                   resultado.opciones.archivoEntrada + "' y '" + argumento + "').";
                return resultado;
            }
        }
    }

    // Validacion: si no se pide ayuda ni version, debe existir un archivo de entrada
    if (!resultado.opciones.mostrarAyuda && !resultado.opciones.mostrarVersion && resultado.opciones.archivoEntrada.empty()) {
        resultado.exito = false;
        resultado.mensajeError = "Error: no se especificó ningún archivo de entrada. Use --help para consultar el uso.";
        return resultado;
    }

    return resultado;
}

void AnalizadorCLI::mostrarAyuda(std::ostream& salida) {
    salida << "Uso: x86_64_to_aarch64 [opciones] [archivo_entrada]\n\n"
           << "Traductor estatico determinista de ensamblador x86-64 a AArch64.\n\n"
           << "Opciones:\n"
           << "  -h, --help               Muestra este mensaje de ayuda y termina.\n"
           << "  -v, --version            Muestra la version del traductor y termina.\n"
           << "  -o, --output <archivo>   Escribe el codigo AArch64 generado en el archivo indicado.\n"
           << "  --explain                Muestra explicaciones didacticas de las reglas aplicadas.\n"
           << "  --dump-ir                Vuelca la representacion intermedia (IR x86-64 y AArch64).\n\n"
           << "Comportamiento por defecto:\n"
           << "  Sin opciones ni archivo: Ejecuta una demostracion interactiva interna.\n"
           << "  Con archivo de entrada:  Emite el codigo ensamblador AArch64 a la salida estandar\n"
           << "                           (o al archivo especificado mediante -o).\n";
}

void AnalizadorCLI::mostrarVersion(std::ostream& salida) {
    salida << "x86_64_to_aarch64 version 0.3.6\n"
           << "Traductor estatico de x86-64 a AArch64 (Fase 3: Nucleo del Traductor)\n";
}

int AnalizadorCLI::ejecutar(const OpcionesCLI& opciones, std::ostream& salida, std::ostream& error) {
    if (opciones.mostrarAyuda) {
        mostrarAyuda(salida);
        return 0;
    }

    if (opciones.mostrarVersion) {
        mostrarVersion(salida);
        return 0;
    }

    if (opciones.ejecutarDemo) {
        salida << "=== Demostracion del Traductor x86-64 a AArch64 ===\n\n";

        std::string programaEjemplo =
            "; Programa x86-64 de prueba\n"
            "inicio:\n"
            "    mov rax, 42       ; valor inicial\n"
            "    mov rbx, 10\n"
            "    add rax, rbx      ; suma de registros\n"
            "    sub rax, 2        ; resta inmediata\n"
            "    ret               ; retornar resultado\n";

        salida << "[Programa x86-64 de Entrada]:\n" << programaEjemplo << "\n";

        ResultadoTraduccion resultado = Traductor::traducir(programaEjemplo);
        salida << resultado.resumenFormateado();
        salida << "\nUso como CLI: ./x86_64_to_aarch64 [opciones] <archivo.s>\n";
        return 0;
    }

    // Lectura del archivo de entrada
    std::ifstream archivoEntrada(opciones.archivoEntrada);
    if (!archivoEntrada.is_open()) {
        error << "Error: no se pudo abrir el archivo de entrada \"" << opciones.archivoEntrada << "\".\n";
        return 1;
    }

    std::stringstream flujoEntrada;
    flujoEntrada << archivoEntrada.rdbuf();
    std::string codigoFuente = flujoEntrada.str();

    ResultadoTraduccion resultado = Traductor::traducir(codigoFuente);

    if (!resultado.esExitoso()) {
        error << "Error en la traduccion de \"" << opciones.archivoEntrada << "\":\n";
        for (const auto& diagnostico : resultado.diagnosticos) {
            error << "  " << diagnostico.formatear() << "\n";
        }
        return 1;
    }

    // Modo --dump-ir: volcado de representaciones intermedias
    if (opciones.volcarIR) {
        salida << resultado.volcadoIR() << "\n";
    }

    // Modo --explain: explicaciones didacticas de reglas aplicadas
    if (opciones.explicar) {
        salida << resultado.explicacionesFormateadas() << "\n";
    }

    // Emision de codigo AArch64
    if (!opciones.archivoSalida.empty()) {
        std::ofstream archivoSalida(opciones.archivoSalida);
        if (!archivoSalida.is_open()) {
            error << "Error: no se pudo crear o escribir en el archivo de salida \"" << opciones.archivoSalida << "\".\n";
            return 1;
        }
        archivoSalida << resultado.codigoGenerado;
        archivoSalida.close();
        salida << "Traduccion completada con exito. Archivo de salida generado: " << opciones.archivoSalida << "\n";
    } else {
        if (opciones.volcarIR || opciones.explicar) {
            salida << "[Codigo AArch64 Generado]:\n" << resultado.codigoGenerado;
        } else {
            salida << resultado.codigoGenerado;
        }
    }

    return 0;
}
