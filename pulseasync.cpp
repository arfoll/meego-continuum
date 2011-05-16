#include "pulseasync.h"
#include <QHash>

QList<u_int32_t> outputSinks;
QList<u_int32_t> inputSinks;
QHash<QString, QString> outputSinksComplex;
QHash<QString, QString> outputSinksSimple;

#define DUPES 1

extern "C" {

static pa_mainloop_api * mainloop_api = NULL;
static char *selectedSink;
static u_int32_t idx = -1;

enum {
    NONE = 0,
    GET_SINKS,
    GET_INPUTS,
    MOVE_SINK,
    SET_DEF_SINK,
} action;

static void quit(int ret)
{
    /* quit the mainloop to return from the run() method */
    mainloop_api->quit(mainloop_api, ret);
}

static void moved_sink_callback(pa_context *c, int ret, void *userdata)
{
    /* we have finished quit the mainloop */
    pa_context_disconnect (c);
}

static void set_default_sink_callback(pa_context *c, int ret, void *userdata)
{
    /* we have finished quit the mainloop */
    pa_context_disconnect (c);
}

static void get_sink_input_info_callback(pa_context *c, const pa_sink_input_info *i, int is_last, void *userdata)
{
    if (is_last < 0) {
        qDebug("Failed to get sink input information");
    }
    else if (is_last) {
        mainloop_api->quit(mainloop_api, 1);
        return;
    }

    inputSinks.append(i->index);
    qDebug("Input Sink #%u", i->index);
}

static void get_sink_info_callback(pa_context *c, const pa_sink_info *i, int is_last, void *userdata)
{
    if (is_last < 0) {
        qDebug("Failed to get sink information");
    }
    else if (is_last) {
        mainloop_api->quit(mainloop_api, 1);
        return;
    }

    outputSinks.append(i->index);
    outputSinksComplex.insert(QString(QLatin1String(i->name)), QString(QLatin1String(pa_proplist_gets (i->proplist, "device.description"))));
    qDebug("Sink #%u", i->index);
    //qDebug("proplist = %s", pa_proplist_to_string_sep(i->proplist, "\n"));
}

static void context_state_callback(pa_context *c, void *userdata)
{
    switch (pa_context_get_state(c)) {
        case PA_CONTEXT_CONNECTING:
        case PA_CONTEXT_AUTHORIZING:
        case PA_CONTEXT_SETTING_NAME:
        case PA_CONTEXT_UNCONNECTED:
        case PA_CONTEXT_FAILED:
            break;
        case PA_CONTEXT_TERMINATED:
            qDebug("quitting mainloop!");
            quit(1);
            break;

        case PA_CONTEXT_READY:
            switch (action) {
                case GET_SINKS:
                    //clear the list
                    outputSinks.clear();
                    pa_operation_unref(pa_context_get_sink_info_list(c, get_sink_info_callback, NULL));
                    qDebug("get sinks");
                    break;
                case GET_INPUTS:
                    //clear the list
                    inputSinks.clear();
                    pa_operation_unref(pa_context_get_sink_input_info_list(c, get_sink_input_info_callback, NULL));
                    qDebug("get sink inputs");
                    break;
                case MOVE_SINK:
                    if (idx != -1) {
                        pa_operation_unref(pa_context_move_sink_input_by_name (c, idx, selectedSink, moved_sink_callback, NULL));
                        qDebug("moved sink input");
                    }
                    break;
                case SET_DEF_SINK:
                    pa_operation_unref(pa_context_set_default_sink (c, selectedSink, set_default_sink_callback, NULL));
                    qDebug("setting default sink");
                    break;
                default:
                    qDebug("error: action unkown");
                    pa_context_disconnect (c);
                    break;
            }
            break;
    }
    return;
}

void setMainloop ()
{
    int ret = 1;
    // PA Server to connect to
    char *server = NULL;
    pa_mainloop* m = NULL;
    pa_context * c;
    pa_proplist * proplist = pa_proplist_new();

    if (!(m = pa_mainloop_new())) {
        qDebug("pa_mainloop_new() failed.");
        return;
    }
    mainloop_api = pa_mainloop_get_api(m);

    if (!(c = pa_context_new_with_proplist(mainloop_api, NULL, proplist))) {
        qDebug("pa_context_new() failed.");
        return;
    }
    pa_context_set_state_callback(c, context_state_callback, NULL);
    if (pa_context_connect(c, server, PA_CONTEXT_NOAUTOSPAWN, NULL) < 0) {
        qDebug("pa_context_connect() failed: %s", pa_strerror(pa_context_errno(c)));
        return;
    }
    if (pa_mainloop_run(m, &ret) < 0) {
        qDebug("pa_mainloop_run() failed.");
        return;
    }
    pa_context_unref(c);
    pa_mainloop_free(m);
    qDebug("Done action type %d", action);
}

void getInputs()
{
    //write all the inputs to the qlist
    action = GET_INPUTS;
    setMainloop();
}

QList<QString> getSinks()
{
    outputSinksSimple.clear();
    //start by clearing the hashtable
    outputSinksComplex.clear();
    //write all the sinks to the qlist outputSinks
    action = GET_SINKS;
    setMainloop();

    int localcount = 0;

    QList<QString> sinkList;
#if DUPES
    //treat doubles here
    QHashIterator<QString,QString> g(outputSinksComplex);
    while (g.hasNext()) {
        g.next();
        QList<QString> dupeCheck = g.value().split("@");
        if (dupeCheck.size() <= 1) {
            localcount++;
        }
    }
#endif
    QHashIterator<QString,QString> j(outputSinksComplex);
    while (j.hasNext()) {
        j.next();
        //qDebug("Refreshing : %s", j.value());
        // check for tunnel instead
        QList<QString> splitName = j.value().split("@");
        if (splitName.size() > 1) {
            outputSinksSimple.insert(j.value(), splitName.at(1));
            sinkList.append(splitName.at(1));
        } else {
            if (localcount <= 1) {
                outputSinksSimple.insert(j.value(), "localhost");
                sinkList.append("localhost");
            } else {
                outputSinksSimple.insert(j.value(), j.value());
                sinkList.append(j.value());
                qDebug("found dupe");
            }
        }
    }

    return sinkList;
}

QString getComplexSink(QString sinkName)
{
    QList<QString> keys;

    // get the simplified name
    keys = outputSinksSimple.keys (sinkName);
    // get the dumb name
    keys = outputSinksComplex.keys (keys.at(0));

    if (keys.size() > 0) {
        return keys.at(0);
    } else {
        qDebug("No keys found for this value");
        return "nothing";
    }
}

void setDefaultOutputSink()
{
    action = SET_DEF_SINK;
    setMainloop();
}

void MoveOuputSink(char *sink)
{
    //set all the inputs in the qlist to the selected sink
    getInputs();
    selectedSink = sink;
    setDefaultOutputSink();

    // move all the input sinks
    action = MOVE_SINK;
    u_int32_t s;
    foreach (s, inputSinks) {
        idx = s;
        setMainloop();
        qDebug ("Moved input : %d", s);
    }
}

}
