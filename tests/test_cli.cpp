#include "opciones_cli.hpp"
#include <iostream>
#include <cassert>
#include <sstream>
#include <fstream>
#include <vector>

void probarParseoAyudaYVersion() {
    std::cout << "[Test CLI] Probando parseo de opciones de ayuda, version y demo...\n";

    // 1. Sin argumentos (modo demostracion)
    {
        char* argv[] = {(char*)"x86_64_to_aarch64"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(1, argv);
        assert(res.exito);
        assert(res.opciones.ejecutarDemo);
        assert(!res.opciones.mostrarAyuda);
        assert(!res.opciones.mostrarVersion);
    }

    // 2. Ayuda con -h y --help
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"-h"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(res.exito);
        assert(res.opciones.mostrarAyuda);
    }
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--help"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(res.exito);
        assert(res.opciones.mostrarAyuda);
    }

    // 3. Version con -v y --version
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"-v"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(res.exito);
        assert(res.opciones.mostrarVersion);
    }
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--version"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(res.exito);
        assert(res.opciones.mostrarVersion);
    }

    std::cout << "  -> Exito en parseo de ayuda, version y demo.\n";
}

void probarParseoBanderasTraduccion() {
    std::cout << "[Test CLI] Probando parseo de archivo y banderas de traduccion...\n";

    // 1. Archivo de entrada basico
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(res.exito);
        assert(res.opciones.archivoEntrada == "programa.s");
        assert(res.opciones.archivoSalida.empty());
        assert(!res.opciones.explicar);
        assert(!res.opciones.volcarIR);
    }

    // 2. Archivo de entrada con salida (-o archivo.s)
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-o", (char*)"salida.s"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(4, argv);
        assert(res.exito);
        assert(res.opciones.archivoEntrada == "programa.s");
        assert(res.opciones.archivoSalida == "salida.s");
    }

    // 3. Salida con sintaxis pegada (-osalida.s)
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-osalida.s"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(3, argv);
        assert(res.exito);
        assert(res.opciones.archivoEntrada == "programa.s");
        assert(res.opciones.archivoSalida == "salida.s");
    }

    // 4. Salida con --output
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--output", (char*)"salida.s", (char*)"programa.s"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(4, argv);
        assert(res.exito);
        assert(res.opciones.archivoEntrada == "programa.s");
        assert(res.opciones.archivoSalida == "salida.s");
    }

    // 5. Banderas --explain y --dump-ir
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"--explain", (char*)"--dump-ir"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(4, argv);
        assert(res.exito);
        assert(res.opciones.archivoEntrada == "programa.s");
        assert(res.opciones.explicar);
        assert(res.opciones.volcarIR);
    }

    std::cout << "  -> Exito en parseo de banderas de traduccion.\n";
}

void probarParseoErrores() {
    std::cout << "[Test CLI] Probando deteccion de errores en argumentos CLI...\n";

    // 1. Opcion desconocida
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--opcion-invalida"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(!res.exito);
        assert(res.mensajeError.find("opción desconocida") != std::string::npos);
    }

    // 2. Falta de argumento para -o
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-o"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(3, argv);
        assert(!res.exito);
        assert(res.mensajeError.find("requiere un argumento") != std::string::npos);
    }

    // 3. Mas de un archivo de entrada
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"prog1.s", (char*)"prog2.s"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(3, argv);
        assert(!res.exito);
        assert(res.mensajeError.find("más de un archivo de entrada") != std::string::npos);
    }

    // 4. Banderas sin archivo de entrada
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--explain"};
        ResultadoParseoCLI res = AnalizadorCLI::parsear(2, argv);
        assert(!res.exito);
        assert(res.mensajeError.find("no se especificó ningún archivo de entrada") != std::string::npos);
    }

    std::cout << "  -> Exito en deteccion de errores de argumentos CLI.\n";
}

void probarEjecucionCLI() {
    std::cout << "[Test CLI] Probando ejecucion y despacho de opciones...\n";

    // 1. Ayuda
    {
        OpcionesCLI opc;
        opc.mostrarAyuda = true;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("Uso: x86_64_to_aarch64") != std::string::npos);
        assert(ssOut.str().find("--explain") != std::string::npos);
        assert(ssOut.str().find("--dump-ir") != std::string::npos);
    }

    // 2. Version
    {
        OpcionesCLI opc;
        opc.mostrarVersion = true;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("version 0.3.6") != std::string::npos);
    }

    // 3. Demostracion
    {
        OpcionesCLI opc;
        opc.ejecutarDemo = true;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("Demostracion del Traductor") != std::string::npos);
        assert(ssOut.str().find("Resultado de Traduccion") != std::string::npos);
    }

    // 4. Archivo inexistente
    {
        OpcionesCLI opc;
        opc.archivoEntrada = "archivo_inexistente_12345.s";
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 1);
        assert(ssErr.str().find("no se pudo abrir") != std::string::npos);
    }

    // 5. Traduccion de archivo temporal valido
    std::string rutaTempEntrada = "temp_cli_prueba_in.s";
    std::string rutaTempSalida = "temp_cli_prueba_out.s";

    {
        std::ofstream archIn(rutaTempEntrada);
        archIn << "inicio:\n"
               << "    mov rdi, 15\n"
               << "    add rdi, 5\n"
               << "    ret\n";
    }

    // 5a. Traduccion directa a flujo de salida estandar
    {
        OpcionesCLI opc;
        opc.archivoEntrada = rutaTempEntrada;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("mov x1, #15") != std::string::npos);
        assert(ssOut.str().find("add x1, x1, #5") != std::string::npos);
        assert(ssOut.str().find("ret") != std::string::npos);
    }

    // 5b. Traduccion con --explain
    {
        OpcionesCLI opc;
        opc.archivoEntrada = rutaTempEntrada;
        opc.explicar = true;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("Explicaciones Didacticas") != std::string::npos);
        assert(ssOut.str().find("ARITH_ADD_IMM") != std::string::npos);
    }

    // 5c. Traduccion con --dump-ir
    {
        OpcionesCLI opc;
        opc.archivoEntrada = rutaTempEntrada;
        opc.volcarIR = true;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("Volcado de Representacion Intermedia") != std::string::npos);
        assert(ssOut.str().find("[IR x86-64 de Entrada (AST)]") != std::string::npos);
        assert(ssOut.str().find("[IR AArch64 de Destino]") != std::string::npos);
    }

    // 5d. Traduccion con -o archivo_salida
    {
        OpcionesCLI opc;
        opc.archivoEntrada = rutaTempEntrada;
        opc.archivoSalida = rutaTempSalida;
        std::stringstream ssOut, ssErr;
        int codigo = AnalizadorCLI::ejecutar(opc, ssOut, ssErr);
        assert(codigo == 0);
        assert(ssOut.str().find("Archivo de salida generado") != std::string::npos);

        // Verificamos el contenido del archivo generado
        std::ifstream archOut(rutaTempSalida);
        assert(archOut.is_open());
        std::stringstream ssArch;
        ssArch << archOut.rdbuf();
        std::string codigoGenerado = ssArch.str();
        assert(codigoGenerado.find("mov x1, #15") != std::string::npos);
        assert(codigoGenerado.find("add x1, x1, #5") != std::string::npos);
        assert(codigoGenerado.find("ret") != std::string::npos);
        archOut.close();
    }

    // Limpieza de archivos temporales
    std::remove(rutaTempEntrada.c_str());
    std::remove(rutaTempSalida.c_str());

    std::cout << "  -> Exito en ejecucion y despacho de opciones del CLI.\n";
}

int main() {
    std::cout << "=== Pruebas de la Interfaz de Linea de Comandos (CLI) ===\n";
    probarParseoAyudaYVersion();
    probarParseoBanderasTraduccion();
    probarParseoErrores();
    probarEjecucionCLI();
    std::cout << "Todas las pruebas del CLI pasaron con exito.\n";
    return 0;
}
