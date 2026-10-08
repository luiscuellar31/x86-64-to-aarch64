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
    Cmp,      // Comparacion y actualizacion de flags NZCV (cmp)
    B,        // Salto incondicional relativo (b)
    BEq,      // Salto condicional si igual / zero (b.eq)
    BNe,      // Salto condicional si no igual / not zero (b.ne)
    BLt,      // Salto condicional si menor con signo (b.lt)
    BLe,      // Salto condicional si menor o igual con signo (b.le)
    BGt,      // Salto condicional si mayor con signo (b.gt)
    BGe,      // Salto condicional si mayor o igual con signo (b.ge)
    Ldr,      // Carga desde memoria a registro (ldr)
    Str,      // Almacenamiento desde registro a memoria (str)
    Bl,       // Salto con enlace a subrutina (bl)
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
    Etiqueta,  // Simbolo de destino de salto o referencia
    Memoria    // Direccionamiento indirecto de memoria (ej. [x29, #-8])
};

/**
 * @brief Modos de indexado en instrucciones de memoria en AArch64.
 */
enum class ModoIndexadoAArch64 {
    Offset,       // Directo con desplazamiento opcional: [base, #offset] o [base]
    PreIndexado,  // Pre-indexado con actualizacion del base (writeback): [base, #offset]!
    PostIndexado  // Post-indexado con actualizacion del base (writeback): [base], #offset
};

/**
 * @brief Representa un operando de memoria en AArch64 (ej. [x29, #-8], [sp, #-16]!, [sp], #16).
 */
struct OperandoMemoriaAArch64 {
    std::string registroBase;
    int64_t desplazamiento;
    ModoIndexadoAArch64 modo;

    OperandoMemoriaAArch64();
    OperandoMemoriaAArch64(const std::string& base, int64_t desp = 0, ModoIndexadoAArch64 modoIndex = ModoIndexadoAArch64::Offset);
    std::string emitirTexto() const;
};

/**
 * @brief Representa un operando individual en la IR de AArch64.
 */
struct OperandoAArch64 {
    TipoOperandoAArch64 tipo;
    std::string valor;
    int64_t valorNumerico;
    OperandoMemoriaAArch64 memoria;

    OperandoAArch64();
    OperandoAArch64(TipoOperandoAArch64 tipoOp, const std::string& valorTexto, int64_t valorNum = 0);
    OperandoAArch64(const OperandoMemoriaAArch64& opMemoria);

    bool esRegistro() const;
    bool esInmediato() const;
    bool esEtiqueta() const;
    bool esMemoria() const;

    static OperandoAArch64 crearRegistro(const std::string& nombreRegistro);
    static OperandoAArch64 crearInmediato(int64_t valorNum);
    static OperandoAArch64 crearInmediato(const std::string& valorTexto, int64_t valorNum);
    static OperandoAArch64 crearEtiqueta(const std::string& nombreEtiqueta);
    static OperandoAArch64 crearMemoria(const std::string& registroBase, int64_t desplazamiento = 0, ModoIndexadoAArch64 modo = ModoIndexadoAArch64::Offset);
    static OperandoAArch64 crearMemoria(const OperandoMemoriaAArch64& opMemoria);

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
