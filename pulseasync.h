#ifndef PULSEASYNC_H
#define PULSEASYNC_H

#include <pulse/pulseaudio.h>
#include <pulse/glib-mainloop.h>
#include <QMainWindow>

extern "C" {
    void MoveOuputSink(char *sink);
    QList<QString> getSinks();
    QString getComplexSink(QString sinkName);
}

#endif // PULSEASYNC_H
