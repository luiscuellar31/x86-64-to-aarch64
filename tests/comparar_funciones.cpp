#include <cstdint>
#include <iostream>
#include <limits>

extern "C" std::int64_t evaluar(std::int64_t primerValor, std::int64_t segundoValor);

struct EntradaFuncional {
    std::int64_t primerValor;
    std::int64_t segundoValor;
};

int main() {
    const std::int64_t minimo = std::numeric_limits<std::int64_t>::min();
    const std::int64_t maximo = std::numeric_limits<std::int64_t>::max();
    const EntradaFuncional entradas[] = {
        {0, 0}, {1, 1}, {-1, -1}, {1, -1}, {-1, 1},
        {7, 3}, {-7, 3}, {7, -3}, {-7, -3}, {255, 15},
        {minimo, 0}, {maximo, 0}, {minimo, maximo},
        {maximo, minimo}, {minimo, minimo}, {maximo, maximo}
    };

    for (int repeticion = 0; repeticion < 3; repeticion++) {
        for (const auto& entrada : entradas) {
            std::int64_t resultado = evaluar(entrada.primerValor, entrada.segundoValor);
            std::cout << entrada.primerValor << " " << entrada.segundoValor << " " << resultado << "\n";
        }
    }
    return 0;
}
