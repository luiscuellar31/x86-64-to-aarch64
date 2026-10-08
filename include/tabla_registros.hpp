#ifndef TABLA_REGISTROS_HPP
#define TABLA_REGISTROS_HPP

#include <string>

/**
 * @brief Comprueba si una cadena representa un registro x86-64 de 64 bits soportado.
 * @param nombre Cadena con el nombre del registro (insensible a mayusculas).
 * @return true si es un registro x86-64 soportado en el subconjunto, false en caso contrario.
 */
bool esRegistroX86(const std::string& nombre);

/**
 * @brief Convierte el nombre de un registro a minusculas estandar.
 * @param nombre Cadena con el nombre del registro.
 * @return Nombre normalizado en minusculas.
 */
std::string normalizarNombreRegistro(const std::string& nombre);

/**
 * @brief Obtiene el registro AArch64 equivalente segun la POLITICA_REGISTROS.
 * @param registroX86 Nombre del registro x86-64 (ej. "rax").
 * @return Nombre del registro AArch64 (ej. "x0") o vacio si no se reconoce.
 */
std::string obtenerEquivalenteAArch64(const std::string& registroX86);

#endif // TABLA_REGISTROS_HPP
