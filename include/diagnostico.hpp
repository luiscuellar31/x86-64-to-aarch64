#ifndef DIAGNOSTICO_HPP
#define DIAGNOSTICO_HPP

#include <string>
#include <vector>
#include <iostream>

/**
 * @brief Nivel de severidad de un diagnóstico emitido por el sistema.
 */
enum class SeveridadDiagnostico {
    Informacion,
    Advertencia,
    Error
};

/**
 * @brief Convierte la severidad a una cadena representativa.
 */
std::string severidadDiagnosticoATexto(SeveridadDiagnostico severidad);

/**
 * @brief Códigos de error y diagnóstico estándar del proyecto.
 */
namespace CodigosDiagnostico {
    inline const std::string kInstruccionDesconocida = "E001";
    inline const std::string kFormaOperandoNoSoportada = "E014";
    inline const std::string kRegistroNoSoportado64Bit = "E021";
    inline const std::string kExcesoOperandos = "E022";
    inline const std::string kFaltanOperandos = "E023";
    inline const std::string kInmediatoInvalido = "E024";
    inline const std::string kSintaxisInvalida = "E030";
    inline const std::string kEtiquetaNoDefinida = "E040";
}

/**
 * @brief Estructura que describe un diagnóstico de compilación o traducción con ubicación precisa.
 */
struct Diagnostico {
    SeveridadDiagnostico severidad;
    std::string codigo;
    int numeroLinea;
    int numeroColumna;
    std::string mensaje;

    Diagnostico();
    Diagnostico(SeveridadDiagnostico nivel, const std::string& codigoError,
                int linea, const std::string& textoMensaje, int columna = 0);

    // Fábricas convenientes y legibles
    static Diagnostico crearError(const std::string& codigoError, int linea,
                                  const std::string& mensaje, int columna = 0);

    static Diagnostico crearAdvertencia(const std::string& codigoError, int linea,
                                        const std::string& mensaje, int columna = 0);

    static Diagnostico crearInformacion(const std::string& codigoError, int linea,
                                        const std::string& mensaje, int columna = 0);

    // Formatea el diagnóstico en formato estándar (ej. "linea 4:1: [E001] ERROR: mensaje")
    std::string formatear() const;
};

/**
 * @brief Contenedor y gestor de la lista de diagnósticos generados durante el análisis o traducción.
 */
class GestorDiagnosticos {
private:
    std::vector<Diagnostico> listaDiagnosticos;

public:
    GestorDiagnosticos();

    void agregar(const Diagnostico& diagnostico);

    void agregarError(const std::string& codigo, int linea,
                      const std::string& mensaje, int columna = 0);

    void agregarAdvertencia(const std::string& codigo, int linea,
                            const std::string& mensaje, int columna = 0);

    void agregarInformacion(const std::string& codigo, int linea,
                            const std::string& mensaje, int columna = 0);

    bool tieneErrores() const;
    bool estaVacio() const;
    size_t cantidadTotal() const;
    size_t cantidadErrores() const;
    size_t cantidadAdvertencias() const;

    const std::vector<Diagnostico>& obtenerTodos() const;
    void limpiar();

    // Imprime todos los diagnósticos registrados en orden
    void imprimir(std::ostream& salida = std::cout) const;
};

#endif // DIAGNOSTICO_HPP
