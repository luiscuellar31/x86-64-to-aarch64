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
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(1, argv);
        assert(resultado.exito);
        assert(resultado.opciones.ejecutarDemo);
        assert(!resultado.opciones.mostrarAyuda);
        assert(!resultado.opciones.mostrarVersion);
    }

    // 2. Ayuda con -h y --help
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"-h"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(resultado.exito);
        assert(resultado.opciones.mostrarAyuda);
    }
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--help"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(resultado.exito);
        assert(resultado.opciones.mostrarAyuda);
    }

    // 3. Version con -v y --version
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"-v"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(resultado.exito);
        assert(resultado.opciones.mostrarVersion);
    }
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--version"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(resultado.exito);
        assert(resultado.opciones.mostrarVersion);
    }

    std::cout << "  -> Exito en parseo de ayuda, version y demo.\n";
}

void probarParseoBanderasTraduccion() {
    std::cout << "[Test CLI] Probando parseo de archivo y banderas de traduccion...\n";

    // 1. Archivo de entrada basico
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(resultado.exito);
        assert(resultado.opciones.archivoEntrada == "programa.s");
        assert(resultado.opciones.archivoSalida.empty());
        assert(!resultado.opciones.explicar);
        assert(!resultado.opciones.volcarIR);
    }

    // 2. Archivo de entrada con salida (-o archivo.s)
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-o", (char*)"salida.s"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(4, argv);
        assert(resultado.exito);
        assert(resultado.opciones.archivoEntrada == "programa.s");
        assert(resultado.opciones.archivoSalida == "salida.s");
    }

    // 3. Salida con sintaxis pegada (-osalida.s)
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-osalida.s"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(3, argv);
        assert(resultado.exito);
        assert(resultado.opciones.archivoEntrada == "programa.s");
        assert(resultado.opciones.archivoSalida == "salida.s");
    }

    // 4. Salida con --output
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--output", (char*)"salida.s", (char*)"programa.s"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(4, argv);
        assert(resultado.exito);
        assert(resultado.opciones.archivoEntrada == "programa.s");
        assert(resultado.opciones.archivoSalida == "salida.s");
    }

    // 5. Banderas --explain y --dump-ir
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"--explain", (char*)"--dump-ir"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(4, argv);
        assert(resultado.exito);
        assert(resultado.opciones.archivoEntrada == "programa.s");
        assert(resultado.opciones.explicar);
        assert(resultado.opciones.volcarIR);
    }

    std::cout << "  -> Exito en parseo de banderas de traduccion.\n";
}

void probarParseoErrores() {
    std::cout << "[Test CLI] Probando deteccion de errores en argumentos CLI...\n";

    // 1. Opcion desconocida
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--opcion-invalida"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(!resultado.exito);
        assert(resultado.mensajeError.find("opción desconocida") != std::string::npos);
    }

    // 2. Falta de argumento para -o
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"programa.s", (char*)"-o"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(3, argv);
        assert(!resultado.exito);
        assert(resultado.mensajeError.find("requiere un argumento") != std::string::npos);
    }

    // 3. Mas de un archivo de entrada
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"prog1.s", (char*)"prog2.s"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(3, argv);
        assert(!resultado.exito);
        assert(resultado.mensajeError.find("más de un archivo de entrada") != std::string::npos);
    }

    // 4. Banderas sin archivo de entrada
    {
        char* argv[] = {(char*)"x86_64_to_aarch64", (char*)"--explain"};
        ResultadoParseoCLI resultado = AnalizadorCLI::parsear(2, argv);
        assert(!resultado.exito);
        assert(resultado.mensajeError.find("no se especificó ningún archivo de entrada") != std::string::npos);
    }

    std::cout << "  -> Exito en deteccion de errores de argumentos CLI.\n";
}

static void probarEjecucionAyuda() {
    OpcionesCLI opciones;
    opciones.mostrarAyuda = true;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("Uso: x86_64_to_aarch64") != std::string::npos);
    assert(flujoSalida.str().find("--explain") != std::string::npos);
    assert(flujoSalida.str().find("--dump-ir") != std::string::npos);
}

static void probarEjecucionVersion() {
    OpcionesCLI opciones;
    opciones.mostrarVersion = true;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("version 0.3.6") != std::string::npos);
}

static void probarEjecucionDemostracion() {
    OpcionesCLI opciones;
    opciones.ejecutarDemo = true;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("Demostracion del Traductor") != std::string::npos);
    assert(flujoSalida.str().find("Resultado de Traduccion") != std::string::npos);
}

static void probarArchivoInexistente() {
    OpcionesCLI opciones;
    opciones.archivoEntrada = "archivo_inexistente_12345.s";
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 1);
    assert(flujoError.str().find("no se pudo abrir") != std::string::npos);
}

static void probarTraduccionPorConsola(const std::string& rutaEntrada) {
    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("mov x1, #15") != std::string::npos);
    assert(flujoSalida.str().find("add x1, x1, #5") != std::string::npos);
    assert(flujoSalida.str().find("ret") != std::string::npos);
}

static void probarExplicacionesCLI(const std::string& rutaEntrada) {
    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    opciones.explicar = true;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("Explicaciones Didacticas") != std::string::npos);
    assert(flujoSalida.str().find("ARITH_ADD_IMM") != std::string::npos);
}

static void probarVolcadoIR(const std::string& rutaEntrada) {
    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    opciones.volcarIR = true;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("Volcado de Representacion Intermedia") != std::string::npos);
    assert(flujoSalida.str().find("[IR x86-64 de Entrada (AST)]") != std::string::npos);
    assert(flujoSalida.str().find("[IR AArch64 de Destino]") != std::string::npos);
}

static void probarArchivoSalida(const std::string& rutaEntrada, const std::string& rutaSalida) {
    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    opciones.archivoSalida = rutaSalida;
    std::stringstream flujoSalida, flujoError;
    int codigo = AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError);
    assert(codigo == 0);
    assert(flujoSalida.str().find("Archivo de salida generado") != std::string::npos);

    // Verificamos el contenido del archivo generado
    std::ifstream archivoSalida(rutaSalida);
    assert(archivoSalida.is_open());
    std::stringstream flujoArchivo;
    flujoArchivo << archivoSalida.rdbuf();
    std::string codigoGenerado = flujoArchivo.str();
    assert(codigoGenerado.find("mov x1, #15") != std::string::npos);
    assert(codigoGenerado.find("add x1, x1, #5") != std::string::npos);
    assert(codigoGenerado.find("ret") != std::string::npos);
    archivoSalida.close();
}

static void probarRechazoSinSobrescribir(const std::string& rutaEntrada, const std::string& rutaSalida) {
    std::ofstream archivoEntrada(rutaEntrada);
    archivoEntrada << "; cabecera\n\nadd rax,,5\n";
    archivoEntrada.close();
    std::ofstream archivoSalida(rutaSalida);
    archivoSalida << "contenido anterior\n";
    archivoSalida.close();

    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    opciones.archivoSalida = rutaSalida;
    std::stringstream flujoSalida, flujoError;
    assert(AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError) == 1);
    assert(flujoSalida.str().empty());
    assert(flujoError.str().find("linea 3: [E030]") != std::string::npos);

    std::ifstream archivoPreservado(rutaSalida);
    std::stringstream contenidoPreservado;
    contenidoPreservado << archivoPreservado.rdbuf();
    assert(contenidoPreservado.str() == "contenido anterior\n");
}

static void probarEtiquetaDuplicadaSinSobrescribir(const std::string& rutaEntrada, const std::string& rutaSalida) {
    std::ofstream archivoEntrada(rutaEntrada);
    archivoEntrada << "inicio: ret\ninicio: ret\n";
    archivoEntrada.close();

    OpcionesCLI opciones;
    opciones.archivoEntrada = rutaEntrada;
    opciones.archivoSalida = rutaSalida;
    std::stringstream flujoSalida, flujoError;
    assert(AnalizadorCLI::ejecutar(opciones, flujoSalida, flujoError) == 1);
    assert(flujoSalida.str().empty());
    assert(flujoError.str().find("linea 2: [E041]") != std::string::npos);

    std::ifstream archivoPreservado(rutaSalida);
    std::stringstream contenidoPreservado;
    contenidoPreservado << archivoPreservado.rdbuf();
    assert(contenidoPreservado.str() == "contenido anterior\n");
}

void probarEjecucionCLI() {
    std::cout << "[Test CLI] Probando ejecucion y despacho de opciones...\n";
    probarEjecucionAyuda();
    probarEjecucionVersion();
    probarEjecucionDemostracion();
    probarArchivoInexistente();

    std::string rutaEntrada = "temp_cli_prueba_in.s";
    std::string rutaSalida = "temp_cli_prueba_out.s";
    {
        std::ofstream archivoEntrada(rutaEntrada);
        archivoEntrada << "inicio:\n"
                       << "    mov rdi, 15\n"
                       << "    add rdi, 5\n"
                       << "    ret\n";
    }

    probarTraduccionPorConsola(rutaEntrada);
    probarExplicacionesCLI(rutaEntrada);
    probarVolcadoIR(rutaEntrada);
    probarArchivoSalida(rutaEntrada, rutaSalida);
    probarRechazoSinSobrescribir(rutaEntrada, rutaSalida);
    probarEtiquetaDuplicadaSinSobrescribir(rutaEntrada, rutaSalida);

    std::remove(rutaEntrada.c_str());
    std::remove(rutaSalida.c_str());
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
