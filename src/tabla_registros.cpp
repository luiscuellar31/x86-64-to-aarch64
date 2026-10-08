#include "tabla_registros.hpp"
#include <cctype>
#include <unordered_map>
#include <unordered_set>

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

// Subregistros x86 conocidos (32-bit, 16-bit, 8-bit) no soportados en el MVP V0
static const std::unordered_set<std::string> kSubregistros = {
    // 32-bit
    "eax", "ebx", "ecx", "edx", "esi", "edi", "ebp", "esp",
    "r8d", "r9d", "r10d", "r11d", "r12d", "r13d", "r14d", "r15d",
    // 16-bit
    "ax", "bx", "cx", "dx", "si", "di", "bp", "sp",
    "r8w", "r9w", "r10w", "r11w", "r12w", "r13w", "r14w", "r15w",
    // 8-bit
    "al", "bl", "cl", "dl", "ah", "bh", "ch", "dh",
    "sil", "dil", "bpl", "spl",
    "r8b", "r9b", "r10b", "r11b", "r12b", "r13b", "r14b", "r15b"
};

bool esRegistroX86(const std::string& nombre) {
    std::string nombreNormalizado = aMinusculasInterno(nombre);
    return kMapeoRegistros.find(nombreNormalizado) != kMapeoRegistros.end();
}

bool esSubregistroX86(const std::string& nombre) {
    std::string nombreNormalizado = aMinusculasInterno(nombre);
    return kSubregistros.find(nombreNormalizado) != kSubregistros.end();
}

bool esRegistroX86Cualquiera(const std::string& nombre) {
    return esRegistroX86(nombre) || esSubregistroX86(nombre);
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
