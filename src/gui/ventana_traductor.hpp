#ifndef VENTANA_TRADUCTOR_HPP
#define VENTANA_TRADUCTOR_HPP

#include <QString>
#include <QWidget>

class QPlainTextEdit;
class QPushButton;

class VentanaTraductor : public QWidget {
public:
    explicit VentanaTraductor(QWidget* padre = nullptr);

    bool cargarArchivo(const QString& ruta);
    bool guardarArchivo(const QString& ruta);

private:
    QPlainTextEdit* entradaX86;
    QPlainTextEdit* salidaAArch64;
    QPlainTextEdit* explicaciones;
    QPushButton* botonGuardar;

    void traducir();
    void limpiar();
    void invalidarResultado();
    void elegirEntrada();
    void elegirSalida();
    void mostrarErrorArchivo(const QString& mensaje);
};

#endif
