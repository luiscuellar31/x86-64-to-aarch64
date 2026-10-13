#include "ventana_traductor.hpp"
#include "traductor.hpp"
#include <QFile>
#include <QFileDialog>
#include <QFontDatabase>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QSaveFile>
#include <QSplitter>
#include <QVBoxLayout>

static QWidget* crearPanel(const QString& titulo, QPlainTextEdit* editor) {
    auto* panel = new QWidget;
    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(0, 0, 0, 0);
    auto* etiqueta = new QLabel(titulo, panel);
    etiqueta->setBuddy(editor);
    layout->addWidget(etiqueta);
    layout->addWidget(editor);
    return panel;
}

VentanaTraductor::VentanaTraductor(QWidget* padre) : QWidget(padre) {
    setWindowTitle("Traductor x86-64 a AArch64");
    resize(1000, 700);

    entradaX86 = new QPlainTextEdit(this);
    entradaX86->setObjectName("entradaX86");
    entradaX86->setPlaceholderText("Escribe o carga código x86-64 en sintaxis Intel/NASM.");
    salidaAArch64 = new QPlainTextEdit(this);
    salidaAArch64->setObjectName("salidaAArch64");
    salidaAArch64->setReadOnly(true);
    salidaAArch64->setPlaceholderText("Aquí se mostrará el código traducido.");
    explicaciones = new QPlainTextEdit(this);
    explicaciones->setObjectName("explicaciones");
    explicaciones->setReadOnly(true);
    explicaciones->setPlaceholderText("Aquí se mostrarán las explicaciones y los diagnósticos.");

    QFont fuenteCodigo = QFontDatabase::systemFont(QFontDatabase::FixedFont);
    entradaX86->setFont(fuenteCodigo);
    salidaAArch64->setFont(fuenteCodigo);
    explicaciones->setFont(fuenteCodigo);
    entradaX86->setLineWrapMode(QPlainTextEdit::NoWrap);
    salidaAArch64->setLineWrapMode(QPlainTextEdit::NoWrap);

    auto* paneles = new QSplitter(Qt::Horizontal, this);
    paneles->addWidget(crearPanel("&x86-64 — entrada", entradaX86));
    paneles->addWidget(crearPanel("&AArch64 — salida", salidaAArch64));
    paneles->setStretchFactor(0, 1);
    paneles->setStretchFactor(1, 1);

    auto* botonTraducir = new QPushButton("&Traducir", this);
    botonTraducir->setObjectName("botonTraducir");
    auto* botonLimpiar = new QPushButton("&Limpiar", this);
    botonLimpiar->setObjectName("botonLimpiar");
    auto* botonCargar = new QPushButton("&Cargar", this);
    botonCargar->setObjectName("botonCargar");
    botonGuardar = new QPushButton("&Guardar", this);
    botonGuardar->setObjectName("botonGuardar");
    botonGuardar->setEnabled(false);

    auto* acciones = new QHBoxLayout;
    acciones->addWidget(botonTraducir);
    acciones->addWidget(botonLimpiar);
    acciones->addWidget(botonCargar);
    acciones->addWidget(botonGuardar);
    acciones->addStretch();

    auto* restricciones = new QLabel(
        "Subconjunto entero de 64 bits. La salida debe revisarse según las restricciones "
        "de flags, pila e inmediatos descritas en la documentación.", this);
    restricciones->setWordWrap(true);
    auto* layout = new QVBoxLayout(this);
    layout->addWidget(restricciones);
    layout->addWidget(paneles, 3);
    layout->addLayout(acciones);
    auto* etiquetaExplicaciones = new QLabel("&Explicaciones y diagnósticos", this);
    etiquetaExplicaciones->setBuddy(explicaciones);
    layout->addWidget(etiquetaExplicaciones);
    layout->addWidget(explicaciones, 1);

    connect(botonTraducir, &QPushButton::clicked, this, &VentanaTraductor::traducir);
    connect(botonLimpiar, &QPushButton::clicked, this, &VentanaTraductor::limpiar);
    connect(botonCargar, &QPushButton::clicked, this, &VentanaTraductor::elegirEntrada);
    connect(botonGuardar, &QPushButton::clicked, this, &VentanaTraductor::elegirSalida);
    connect(entradaX86, &QPlainTextEdit::textChanged, this, &VentanaTraductor::invalidarResultado);
}

void VentanaTraductor::invalidarResultado() {
    salidaAArch64->clear();
    explicaciones->clear();
    botonGuardar->setEnabled(false);
}

void VentanaTraductor::traducir() {
    ResultadoTraduccion resultado = Traductor::traducir(entradaX86->toPlainText().toStdString());
    invalidarResultado();
    if (resultado.esExitoso()) {
        salidaAArch64->setPlainText(QString::fromStdString(resultado.codigoGenerado));
        explicaciones->setPlainText(QString::fromStdString(resultado.explicacionesFormateadas()));
        botonGuardar->setEnabled(!resultado.codigoGenerado.empty());
    }
    for (const auto& diagnostico : resultado.diagnosticos) {
        explicaciones->appendPlainText(QString::fromStdString(diagnostico.formatear()));
    }
}

void VentanaTraductor::limpiar() {
    entradaX86->selectAll();
    entradaX86->insertPlainText(QString());
    invalidarResultado();
    entradaX86->setFocus();
}

void VentanaTraductor::elegirEntrada() {
    QString ruta = QFileDialog::getOpenFileName(this, "Cargar código x86-64", QString(),
        "Ensamblador (*.s *.asm);;Todos los archivos (*)");
    if (!ruta.isEmpty()) {
        cargarArchivo(ruta);
    }
}

void VentanaTraductor::elegirSalida() {
    QString ruta = QFileDialog::getSaveFileName(this, "Guardar código AArch64", QString(),
        "Ensamblador (*.s *.asm);;Todos los archivos (*)");
    if (!ruta.isEmpty()) {
        guardarArchivo(ruta);
    }
}

void VentanaTraductor::mostrarErrorArchivo(const QString& mensaje) {
    explicaciones->appendPlainText(mensaje);
}

bool VentanaTraductor::cargarArchivo(const QString& ruta) {
    QFile archivo(ruta);
    if (!archivo.open(QIODevice::ReadOnly)) {
        mostrarErrorArchivo("No se pudo cargar el archivo: " + archivo.errorString());
        return false;
    }
    QByteArray contenido = archivo.readAll();
    if (archivo.error() != QFileDevice::NoError) {
        mostrarErrorArchivo("No se pudo leer el archivo: " + archivo.errorString());
        return false;
    }
    entradaX86->selectAll();
    entradaX86->insertPlainText(QString::fromUtf8(contenido));
    return true;
}

bool VentanaTraductor::guardarArchivo(const QString& ruta) {
    if (!botonGuardar->isEnabled()) {
        mostrarErrorArchivo("Traduce la entrada actual antes de guardar una salida.");
        return false;
    }
    QSaveFile archivo(ruta);
    if (!archivo.open(QIODevice::WriteOnly)) {
        mostrarErrorArchivo("No se pudo guardar el archivo: " + archivo.errorString());
        return false;
    }
    QByteArray contenido = salidaAArch64->toPlainText().toUtf8();
    if (archivo.write(contenido) != contenido.size()) {
        mostrarErrorArchivo("No se pudo escribir el archivo: " + archivo.errorString());
        return false;
    }
    if (!archivo.commit()) {
        mostrarErrorArchivo("No se pudo finalizar el guardado: " + archivo.errorString());
        return false;
    }
    return true;
}
