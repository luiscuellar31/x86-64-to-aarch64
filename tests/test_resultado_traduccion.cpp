#include <iostream>
#include <cassert>
#include "resultado_traduccion.hpp"

void probarResultadoExitosoConMapeos() {
    std::cout << "[Test] Probando ResultadoTraduccion exitoso con mapeos didacticos...\n";

    ResultadoTraduccion resultado;
    assert(resultado.esExitoso());
    assert(!resultado.tieneErrores());

    resultado.codigoGenerado = "mov x0, #42\nret\n";

    MapeoTraduccion mapeo1(1, "MOV_REG_IMM", "Carga de constante inmediata en registro de 64 bits");
    mapeo1.lineasDestinoAArch64 = {1};
    mapeo1.codigoOrigen = "mov rax, 42";
    mapeo1.codigoDestino = "mov x0, #42";
    resultado.agregarMapeo(mapeo1);

    MapeoTraduccion mapeo2(2, "RET_SIMPLE", "Retorno de subrutina");
    mapeo2.lineasDestinoAArch64 = {2};
    mapeo2.codigoOrigen = "ret";
    mapeo2.codigoDestino = "ret";
    resultado.agregarMapeo(mapeo2);

    assert(resultado.esExitoso());
    assert(resultado.mapeos.size() == 2);

    // Consulta por linea x86
    const MapeoTraduccion* encontrado = resultado.buscarMapeoPorLineaX86(1);
    assert(encontrado != nullptr);
    assert(encontrado->reglaId == "MOV_REG_IMM");

    const MapeoTraduccion* noEncontrado = resultado.buscarMapeoPorLineaX86(99);
    assert(noEncontrado == nullptr);

    std::cout << "  -> Exito en resultado y consulta de mapeos.\n";
}

void probarResultadoConErrores() {
    std::cout << "[Test] Probando ResultadoTraduccion con diagnósticos de error...\n";

    ResultadoTraduccion resultado;
    resultado.agregarDiagnostico(Diagnostico::crearError("E001", 4, "instruccion no soportada"));

    assert(resultado.tieneErrores());
    assert(!resultado.esExitoso());
    assert(resultado.diagnosticos.size() == 1);

    std::string resumen = resultado.resumenFormateado();
    assert(resumen.find("CON ERRORES") != std::string::npos);
    assert(resumen.find("E001") != std::string::npos);

    std::cout << "  -> Exito en deteccion de errores en resultado.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de ResultadoTraduccion (Fase 2, Subfase 6) ===\n";
    probarResultadoExitosoConMapeos();
    probarResultadoConErrores();
    std::cout << "Todas las pruebas de resultado de traduccion pasaron con exito.\n";
    return 0;
}
