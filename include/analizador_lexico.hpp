#ifndef ANALIZADOR_LEXICO_HPP
#define ANALIZADOR_LEXICO_HPP

#include <string>
#include <vector>

/**
 * @brief Tipos de tokens léxicos reconocidos en ensamblador x86-64.
 */
enum class TipoToken {
    Desconocido,
    Identificador, // Mnemonicos o nombres de etiquetas
    Registro,      // Registros x86-64 soportados (rax, rbx, etc.)
    Inmediato,     // Constantes numericas (decimales o hexadecimales)
    Coma,          // ','
    DosPuntos,     // ':'
    FinDeLinea     // Fin de instruccion o linea
};

/**
 * @brief Convierte un TipoToken a su representacion textual descriptiva.
 */
std::string tipoTokenATexto(TipoToken tipo);

/**
 * @brief Representa un token individual con su valor y posicion en el codigo fuente.
 */
struct Token {
    TipoToken tipo;
    std::string texto;
    int numeroLinea;
    int columna;

    Token();
    Token(TipoToken tipoToken, const std::string& valorTexto, int linea = 1, int col = 1);

    bool es(TipoToken tipoEsperado) const;
    std::string aTexto() const;
};

/**
 * @brief Analizador lexico (Tokenizer/Lexer) para ensamblador x86-64 simplificado.
 */
class AnalizadorLexico {
public:
    /**
     * @brief Convierte una linea de texto en una secuencia de tokens.
     * @param textoLinea Texto a analizar.
     * @param numeroLinea Numero de linea original.
     * @return Vector de tokens reconocidos.
     */
    static std::vector<Token> tokenizarLinea(const std::string& textoLinea, int numeroLinea = 1);

    /**
     * @brief Convierte un programa completo en una secuencia de tokens con marcadores FinDeLinea.
     * @param codigoFuente Texto completo del programa.
     * @return Vector con todos los tokens del programa.
     */
    static std::vector<Token> tokenizarPrograma(const std::string& codigoFuente);
};

#endif // ANALIZADOR_LEXICO_HPP
