#ifndef IR_AARCH64_HPP
#define IR_AARCH64_HPP

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Códigos de operación para la representación intermedia de AArch64.
 */
enum class CodigoOperacionAArch64 {
    Desconocido,
    Etiqueta, // Marcador de etiqueta (ej. "inicio:")
    Mov,      // Copia o carga (mov)
    Add,      // Suma con destino explicito (add)
    Sub,      // Resta con destino explicito (sub)
    And,      // Operacion logica AND (and)
    Orr,      // Operacion logica OR (orr)
    Eor,      // Operacion logica XOR (eor)
    Ret       // Retorno de funcion (ret)
};

/**
 * @brief Convierte un CodigoOperacionAArch64 a su mnemónico estándar en texto.
 */
std::string codigoOperacionAArch64ATexto(CodigoOperacionAArch64 codigo);

/**
 * @brief Tipos de operandos en instrucciones AArch64.
 */
enum class TipoOperandoAArch64 {
    Desconocido,
    Registro,  // Registros de 64 bits (x0-x30, sp, xzr)
    Inmediato, // Constante con prefijo '#' (ej. #5, #0x1f)
    Etiqueta   // Simbolo de destino de salto o referencia
};

/**
 * @brief Representa un operando individual en la IR de AArch64.
 */
struct OperandoAArch64 {
    TipoOperandoAArch64 tipo;
    std::string valor;
    int64_t valorNumerico;

    OperandoAArch64();
    OperandoAArch64(TipoOperandoAArch64 tipoOp, const std::string& valorTexto, int64_t valorNum = 0);

    bool esRegistro() const;
    bool esInmediato() const;
    bool esEtiqueta() const;

    static OperandoAArch64 crearRegistro(const std::string& nombreRegistro);
    static OperandoAArch64 crearInmediato(int64_t valorNum);
    static OperandoAArch64 crearInmediato(const std::string& valorTexto, int64_t valorNum);
    static OperandoAArch64 crearEtiqueta(const std::string& nombreEtiqueta);

    // Formatea el operando para su emision en ensamblador AArch64 (ej. anteponiendo '#' a inmediatos)
    std::string emitirTexto() const;
};

/**
 * @brief Instrucción semántica intermedia en la arquitectura de destino AArch64.
 */
struct InstruccionAArch64 {
    CodigoOperacionAArch64 codigoOperacion;
    std::vector<OperandoAArch64> operandos;
    std::string etiqueta;
    int lineaOrigen;
    std::string reglaId;      // Identificador de regla didáctica (ej. "ARITH_ADD_IMM")
    std::string explicacion;  // Justificación pedagógica de la transformación

    InstruccionAArch64(CodigoOperacionAArch64 codigo, int linea = 1);

    bool esEtiquetaPura() const;
    void agregarOperando(const OperandoAArch64& operando);

    // Emite la linea formateada en ensamblador AArch64
    std::string emitirTexto() const;
};

#endif // IR_AARCH64_HPP
