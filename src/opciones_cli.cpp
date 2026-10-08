#include "opciones_cli.hpp"
#include "traductor.hpp"
#include <fstream>
#include <sstream>

ResultadoParseoCLI AnalizadorCLI::parsear(int argc, char* argv[]) {
    ResultadoParseoCLI res;

    // Sin argumentos se ejecuta la demostracion interactiva por defecto
    if (argc <= 1) {
        res.opciones.ejecutarDemo = true;
        return res;
    }

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            res.opciones.mostrarAyuda = true;
        } else if (arg == "-v" || arg == "--version") {
            res.opciones.mostrarVersion = true;
        } else if (arg == "--explain") {
            res.opciones.explicar = true;
        } else if (arg == "--dump-ir") {
            res.opciones.volcarIR = true;
        } else if (arg == "-o" || arg == "--output") {
            if (i + 1 < argc) {
                res.opciones.archivoSalida = argv[++i];
            } else {
                res.exito = false;
                res.mensajeError = "Error: la opción '" + arg + "' requiere un argumento con la ruta del archivo.";
                return res;
            }
        } else if (arg.rfind("-o", 0) == 0 && arg.size() > 2) {
            // Soporte para forma pegada: -oarchivo.s
            res.opciones.archivoSalida = arg.substr(2);
        } else if (!arg.empty() && arg[0] == '-') {
            res.exito = false;
            res.mensajeError = "Error: opción desconocida '" + arg + "'. Use --help para consultar las opciones disponibles.";
            return res;
        } else {
            // Argumento posicional (archivo de entrada)
            if (res.opciones.archivoEntrada.empty()) {
                res.opciones.archivoEntrada = arg;
            } else {
                res.exito = false;
                res.mensajeError = "Error: se especificó más de un archivo de entrada ('" +
                                   res.opciones.archivoEntrada + "' y '" + arg + "').";
                return res;
            }
        }
    }

    // Validacion: si no se pide ayuda ni version, debe existir un archivo de entrada
    if (!res.opciones.mostrarAyuda && !res.opciones.mostrarVersion && res.opciones.archivoEntrada.empty()) {
        res.exito = false;
        res.mensajeError = "Error: no se especificó ningún archivo de entrada. Use --help para consultar el uso.";
        return res;
    }

    return res;
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
    std::ifstream archivo(opciones.archivoEntrada);
    if (!archivo.is_open()) {
        error << "Error: no se pudo abrir el archivo de entrada \"" << opciones.archivoEntrada << "\".\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << archivo.rdbuf();
    std::string contenido = buffer.str();

    ResultadoTraduccion resultado = Traductor::traducir(contenido);

    if (!resultado.esExitoso()) {
        error << "Error en la traduccion de \"" << opciones.archivoEntrada << "\":\n";
        for (const auto& diag : resultado.diagnosticos) {
            error << "  " << diag.formatear() << "\n";
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
