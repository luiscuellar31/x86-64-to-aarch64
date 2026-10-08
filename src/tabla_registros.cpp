#include "tabla_registros.hpp"
#include <cctype>
#include <unordered_map>

static std::string aMinusculasInterno(const std::string& texto) {
    std::string resultado = "";
    for (char caracter : texto) {
        resultado += static_cast<char>(std::tolower(static_cast<unsigned char>(caracter)));
    }
    return resultado;
}

// Mapa con la correspondencia inyectiva definida en POLITICA_REGISTROS.md
static const std::unordered_map<std::string, std::string> kMapeoRegistros = {
    {"rax", "x0"},
    {"rdi", "x1"},
    {"rsi", "x2"},
    {"rdx", "x3"},
    {"rcx", "x4"},
    {"r8",  "x5"},
    {"r9",  "x6"},
    {"r10", "x7"},
    {"r11", "x8"},
    {"rbx", "x19"},
    {"r12", "x20"},
    {"r13", "x21"},
    {"r14", "x22"},
    {"r15", "x23"},
    {"rbp", "x29"},
    {"rsp", "sp"}
};

bool esRegistroX86(const std::string& nombre) {
    std::string nombreNormalizado = aMinusculasInterno(nombre);
    return kMapeoRegistros.find(nombreNormalizado) != kMapeoRegistros.end();
}

std::string normalizarNombreRegistro(const std::string& nombre) {
    return aMinusculasInterno(nombre);
}

std::string obtenerEquivalenteAArch64(const std::string& registroX86) {
    std::string nombreNormalizado = aMinusculasInterno(registroX86);
    auto iterador = kMapeoRegistros.find(nombreNormalizado);
    if (iterador != kMapeoRegistros.end()) {
        return iterador->second;
    }
    return "";
}
