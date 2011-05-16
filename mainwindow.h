#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>

namespace Ui {
    class MainWindow;
}
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

private slots:
        void refreshSinks();
        void switcheroo();
        void updateRecordsSink(const QList<QString> &list);
};

#endif // MAINWINDOW_H
