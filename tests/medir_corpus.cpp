#include "traductor.hpp"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

struct GrupoCorpus {
    std::string carpeta;
    bool debeSerAceptado;
};

static std::vector<std::filesystem::path> listarProgramas(const std::filesystem::path& carpeta) {
    std::vector<std::filesystem::path> archivos;
    for (const auto& entrada : std::filesystem::directory_iterator(carpeta)) {
        if (entrada.is_regular_file() && entrada.path().extension() == ".s") {
            archivos.push_back(entrada.path());
        }
    }
    if (archivos.empty()) {
        throw std::runtime_error("el corpus no contiene programas: " + carpeta.string());
    }
    std::sort(archivos.begin(), archivos.end());
    return archivos;
}

static std::string leerCodigo(const std::filesystem::path& ruta) {
    std::ifstream archivo(ruta);
    if (!archivo.is_open()) {
        throw std::runtime_error("no se pudo leer " + ruta.string());
    }
    std::stringstream codigo;
    codigo << archivo.rdbuf();
    if (archivo.bad()) {
        throw std::runtime_error("fallo de lectura en " + ruta.string());
    }
    return codigo.str();
}

static size_t contarOrigen(const std::vector<Instruccion>& instrucciones) {
    size_t cantidad = 0;
    for (const auto& instruccion : instrucciones) {
        if (!instruccion.esEtiquetaPura()) {
            cantidad++;
        }
    }
    return cantidad;
}

static size_t contarDestino(const std::vector<InstruccionAArch64>& instrucciones) {
    size_t cantidad = 0;
    for (const auto& instruccion : instrucciones) {
        if (!instruccion.esEtiquetaPura()) {
            cantidad++;
        }
    }
    return cantidad;
}

static std::string formatearDiagnosticos(const ResultadoTraduccion& resultado) {
    std::string texto;
    for (const auto& diagnostico : resultado.diagnosticos) {
        if (!texto.empty()) {
            texto += "; ";
        }
        texto += diagnostico.codigo + ":" + std::to_string(diagnostico.numeroLinea);
    }
    if (texto.empty()) {
        return "-";
    }
    return texto;
}

static bool medirCorpus(const std::filesystem::path& raiz) {
    const GrupoCorpus grupos[] = {
        {"examples/validos", true},
        {"examples/invalidos", false},
        {"tests/funcionales", true}
    };
    size_t aceptados = 0;
    size_t rechazados = 0;
    bool estadosCoinciden = true;

    std::cout << "# Medición del corpus\n\n"
              << "| Programa | Esperado | Observado | x86 (AST) | AArch64 (IR) | Diagnósticos (código:línea) |\n"
              << "| --- | --- | --- | ---: | ---: | --- |\n";

    for (const auto& grupo : grupos) {
        std::string esperado = "rechazado";
        if (grupo.debeSerAceptado) {
            esperado = "aceptado";
        }
        for (const auto& ruta : listarProgramas(raiz / grupo.carpeta)) {
            ResultadoTraduccion resultado = Traductor::traducir(leerCodigo(ruta));
            bool seAcepto = resultado.esExitoso();
            std::string observado = "rechazado";
            if (seAcepto) {
                observado = "aceptado";
                aceptados++;
            } else {
                rechazados++;
            }
            if (seAcepto != grupo.debeSerAceptado) {
                estadosCoinciden = false;
            }
            std::string programa = grupo.carpeta + "/" + ruta.filename().string();
            std::cout << "| " << programa << " | " << esperado << " | " << observado
                      << " | " << contarOrigen(resultado.irOrigen)
                      << " | " << contarDestino(resultado.irDestino)
                      << " | " << formatearDiagnosticos(resultado) << " |\n";
        }
    }
    std::cout << "\nAceptados: " << aceptados << ". Rechazados: " << rechazados << ".\n";
    return estadosCoinciden;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: medir_corpus <raiz_del_proyecto>\n";
        return 2;
    }
    try {
        if (!medirCorpus(argv[1])) {
            std::cerr << "El estado observado de algún programa no coincide con su clasificación.\n";
            return 1;
        }
    } catch (const std::exception& error) {
        std::cerr << "Error al medir el corpus: " << error.what() << "\n";
        return 1;
    }
    return 0;
}
