#include "opciones_cli.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    ResultadoParseoCLI parseo = AnalizadorCLI::parsear(argc, argv);
    if (!parseo.exito) {
        std::cerr << parseo.mensajeError << "\n";
        return 1;
    }

    return AnalizadorCLI::ejecutar(parseo.opciones, std::cout, std::cerr);
}