#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pulseasync.h"
#include <QHash>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->treeWidget->setHeaderLabels(QStringList() << tr("Available PulseAudio Sinks"));

    /* Connect buttons */
    connect(ui->btReset, SIGNAL(clicked()), this, SLOT(refreshSinks()));
    connect(ui->btMove, SIGNAL(clicked()), this, SLOT(switcheroo()));

    refreshSinks();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::switcheroo()
{
    QList<QTreeWidgetItem *> selectedItems = ui->treeWidget->selectedItems();
    if(!selectedItems.isEmpty()) {
        /* do the conversion here to avoid loosing the actual pointer location */
        QString complexSinkName = getComplexSink(selectedItems.at(0)->text(0));
        QByteArray ba = complexSinkName.toLatin1();
        char * server = ba.data();

        MoveOuputSink(server);
    }
}

void MainWindow::refreshSinks()
{
    QList<QString> sinks = getSinks();
    updateRecordsSink(sinks);
}

void MainWindow::updateRecordsSink(const QList<QString> &list)
{
    ui->treeWidget->clear();
    foreach (QString record, list) {
        new QTreeWidgetItem(ui->treeWidget, QStringList() << record);
    }
}
