#include <iostream>
#include <cassert>
#include "diagnostico.hpp"

void probarFormateoDiagnostico() {
    std::cout << "[Test] Probando creacion y formateo de Diagnostico...\n";

    // 1. Error con linea y columna
    Diagnostico diagCol = Diagnostico::crearError(
        CodigosDiagnostico::kInstruccionDesconocida, 4, "instruccion desconocida 'vmovaps'", 5);

    assert(diagCol.severidad == SeveridadDiagnostico::Error);
    assert(diagCol.codigo == "E001");
    assert(diagCol.numeroLinea == 4);
    assert(diagCol.numeroColumna == 5);
    assert(diagCol.formatear() == "linea 4:5: [E001] ERROR: instruccion desconocida 'vmovaps'");

    // 2. Advertencia sin columna
    Diagnostico diagSinCol = Diagnostico::crearAdvertencia(
        "W002", 10, "etiqueta declarada pero no utilizada");

    assert(diagSinCol.severidad == SeveridadDiagnostico::Advertencia);
    assert(diagSinCol.numeroColumna == 0);
    assert(diagSinCol.formatear() == "linea 10: [W002] ADVERTENCIA: etiqueta declarada pero no utilizada");

    std::cout << "  -> Exito en formateo de diagnosticos.\n";
}

void probarGestorDiagnosticos() {
    std::cout << "[Test] Probando GestorDiagnosticos...\n";

    GestorDiagnosticos gestor;
    assert(gestor.estaVacio());
    assert(!gestor.tieneErrores());
    assert(gestor.cantidadTotal() == 0);

    // Agregar una advertencia
    gestor.agregarAdvertencia("W001", 1, "directiva ignorada");
    assert(!gestor.tieneErrores());
    assert(gestor.cantidadAdvertencias() == 1);
    assert(gestor.cantidadErrores() == 0);

    // Agregar dos errores
    gestor.agregarError(CodigosDiagnostico::kRegistroNoSoportado64Bit, 2, "registro 'eax' no soportado en modo 64 bits", 5);
    gestor.agregarError(CodigosDiagnostico::kExcesoOperandos, 3, "demasiados operandos para 'add'");

    assert(gestor.tieneErrores());
    assert(gestor.cantidadTotal() == 3);
    assert(gestor.cantidadErrores() == 2);
    assert(gestor.cantidadAdvertencias() == 1);

    // Verificar lista obtenida
    const auto& lista = gestor.obtenerTodos();
    assert(lista.size() == 3);
    assert(lista[1].codigo == "E021");

    // Limpiar
    gestor.limpiar();
    assert(gestor.estaVacio());
    assert(!gestor.tieneErrores());

    std::cout << "  -> Exito en comportamiento de GestorDiagnosticos.\n";
}

int main() {
    std::cout << "=== Pruebas unitarias de Diagnosticos Estructurados (Fase 2, Subfase 5) ===\n";
    probarFormateoDiagnostico();
    probarGestorDiagnosticos();
    std::cout << "Todas las pruebas de diagnosticos pasaron con exito.\n";
    return 0;
}
