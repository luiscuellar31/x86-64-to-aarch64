#include "ventana_traductor.hpp"
#include <QApplication>
#include <QFile>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QTemporaryDir>
#include <cassert>
#include <iostream>

static QByteArray leerArchivo(const QString& ruta) {
    QFile archivo(ruta);
    assert(archivo.open(QIODevice::ReadOnly));
    QByteArray contenido = archivo.readAll();
    assert(archivo.error() == QFileDevice::NoError);
    return contenido;
}

static void escribirArchivo(const QString& ruta, const QByteArray& contenido) {
    QFile archivo(ruta);
    assert(archivo.open(QIODevice::WriteOnly));
    assert(archivo.write(contenido) == contenido.size());
}

int main(int argc, char* argv[]) {
    QApplication aplicacion(argc, argv);
    VentanaTraductor ventana;
    auto* entrada = ventana.findChild<QPlainTextEdit*>("entradaX86");
    auto* salida = ventana.findChild<QPlainTextEdit*>("salidaAArch64");
    auto* explicaciones = ventana.findChild<QPlainTextEdit*>("explicaciones");
    auto* traducir = ventana.findChild<QPushButton*>("botonTraducir");
    auto* limpiar = ventana.findChild<QPushButton*>("botonLimpiar");
    auto* cargar = ventana.findChild<QPushButton*>("botonCargar");
    auto* guardar = ventana.findChild<QPushButton*>("botonGuardar");
    assert(entrada && salida && explicaciones && traducir && limpiar && cargar && guardar);
    assert(!entrada->isReadOnly());
    assert(salida->isReadOnly() && explicaciones->isReadOnly());
    assert(entrada->toPlainText().isEmpty() && salida->toPlainText().isEmpty());
    assert(!guardar->isEnabled());

    entrada->setPlainText("mov rax, 42\nadd rax, 8\nret\n");
    traducir->click();
    const QString codigoEsperado = "    mov x0, #42\n    add x0, x0, #8\n    ret\n";
    assert(salida->toPlainText() == codigoEsperado);
    assert(explicaciones->toPlainText().contains("MOV_REG_IMM"));
    assert(explicaciones->toPlainText().contains("ARITH_ADD_IMM"));
    assert(guardar->isEnabled());

    QTemporaryDir carpeta;
    assert(carpeta.isValid());
    QString rutaSalida = carpeta.filePath("salida árm.s");
    assert(ventana.guardarArchivo(rutaSalida));
    assert(leerArchivo(rutaSalida) == codigoEsperado.toUtf8());

    assert(!ventana.cargarArchivo(carpeta.filePath("ausente.s")));
    assert(entrada->toPlainText() == "mov rax, 42\nadd rax, 8\nret\n");
    assert(salida->toPlainText() == codigoEsperado);
    assert(guardar->isEnabled());
    assert(explicaciones->toPlainText().contains("No se pudo cargar"));

    assert(!ventana.guardarArchivo(carpeta.path()));
    assert(salida->toPlainText() == codigoEsperado);
    assert(leerArchivo(rutaSalida) == codigoEsperado.toUtf8());
    assert(explicaciones->toPlainText().contains("No se pudo guardar"));

    entrada->appendPlainText("; cambio de entrada");
    assert(salida->toPlainText().isEmpty());
    assert(explicaciones->toPlainText().isEmpty());
    assert(!guardar->isEnabled());
    assert(!ventana.guardarArchivo(rutaSalida));
    assert(leerArchivo(rutaSalida) == codigoEsperado.toUtf8());

    entrada->setPlainText("; comentario\nmov eax, 1\nret\n");
    traducir->click();
    assert(salida->toPlainText().isEmpty());
    assert(!guardar->isEnabled());
    assert(explicaciones->toPlainText().contains("linea 2: [E021]"));

    QString rutaEntrada = carpeta.filePath("entrada x86.s");
    escribirArchivo(rutaEntrada, "; Comentario\nmov rax, 7\nret\n");
    assert(ventana.cargarArchivo(rutaEntrada));
    assert(entrada->toPlainText() == "; Comentario\nmov rax, 7\nret\n");
    assert(salida->toPlainText().isEmpty() && explicaciones->toPlainText().isEmpty());
    assert(!guardar->isEnabled());
    entrada->undo();
    assert(entrada->toPlainText() == "; comentario\nmov eax, 1\nret\n");
    entrada->redo();
    assert(entrada->toPlainText() == "; Comentario\nmov rax, 7\nret\n");
    traducir->click();
    assert(salida->toPlainText() == "    mov x0, #7\n    ret\n");
    assert(guardar->isEnabled());
    assert(ventana.guardarArchivo(rutaSalida));
    assert(leerArchivo(rutaSalida) == "    mov x0, #7\n    ret\n");

    limpiar->click();
    assert(entrada->toPlainText().isEmpty() && salida->toPlainText().isEmpty());
    assert(explicaciones->toPlainText().isEmpty() && !guardar->isEnabled());
    entrada->undo();
    assert(entrada->toPlainText() == "; Comentario\nmov rax, 7\nret\n");
    assert(salida->toPlainText().isEmpty() && !guardar->isEnabled());
    limpiar->click();
    traducir->click();
    assert(salida->toPlainText().isEmpty() && !guardar->isEnabled());

    if (argc == 2) {
        entrada->setPlainText("mov rax, 42\nadd rax, 8\nret\n");
        traducir->click();
        ventana.show();
        aplicacion.processEvents();
        assert(ventana.grab().save(QString::fromLocal8Bit(argv[1])));
    }
    std::cout << "Doble panel, traducción, diagnósticos y archivos comprobados.\n";
    return 0;
}
