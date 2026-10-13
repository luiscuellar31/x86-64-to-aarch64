#include "ventana_traductor.hpp"
#include <QApplication>

int main(int argc, char* argv[]) {
    QApplication aplicacion(argc, argv);
    VentanaTraductor ventana;
    ventana.show();
    return aplicacion.exec();
}
