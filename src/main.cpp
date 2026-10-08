#include <iostream>
#include <fstream>
#include <sstream>
#include "traductor.hpp"

static void ejecutarDemostracion() {
    std::cout << "=== Demostracion del Traductor x86-64 a AArch64 ===\n\n";

    std::string programaEjemplo =
        "; Programa x86-64 de prueba\n"
        "inicio:\n"
        "    mov rax, 42       ; valor inicial\n"
        "    mov rbx, 10\n"
        "    add rax, rbx      ; suma de registros\n"
        "    sub rax, 2        ; resta inmediata\n"
        "    ret               ; retornar resultado\n";

    std::cout << "[Programa x86-64 de Entrada]:\n" << programaEjemplo << "\n";

    ResultadoTraduccion resultado = Traductor::traducir(programaEjemplo);
    std::cout << resultado.resumenFormateado();

    std::cout << "\nUso como CLI: ./x86_64_to_aarch64 <archivo.s>\n";
}

static int procesarArchivoEntrada(const std::string& rutaArchivo) {
    std::ifstream archivo(rutaArchivo);
    if (!archivo.is_open()) {
        std::cerr << "Error: no se pudo abrir el archivo \"" << rutaArchivo << "\".\n";
        return 1;
    }

    std::stringstream buffer;
    buffer << archivo.rdbuf();
    std::string contenido = buffer.str();

    std::cout << "Traduciendo archivo: " << rutaArchivo << " ...\n\n";

    ResultadoTraduccion resultado = Traductor::traducir(contenido);
    std::cout << resultado.resumenFormateado();

    return resultado.esExitoso() ? 0 : 1;
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string argumento = argv[1];
        if (argumento == "-h" || argumento == "--help") {
            std::cout << "Uso: x86_64_to_aarch64 [opciones] [archivo.s]\n\n"
                      << "Opciones:\n"
                      << "  -h, --help    Muestra esta ayuda\n"
                      << "  Sin opciones  Ejecuta la demostracion interna del traductor\n";
            return 0;
        }
        return procesarArchivoEntrada(argumento);
    }

    ejecutarDemostracion();
    return 0;
}