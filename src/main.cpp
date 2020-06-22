#include "mainwindow.h"

#include <QtGlobal>
#include <QApplication>

// Log
void myMessageOutput(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    time_t rawtime;
    time (&rawtime);
    char* t = ctime(&rawtime);
    t[strlen(t)-1] = '\0';

    switch (type) {
    case QtDebugMsg:
        fprintf(stderr, "[Log %s]: %s (%s:%u, %s)\n", t, localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "[Info %s]: %s (%s:%u, %s)\n", t, localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "[Warning %s]: %s (%s:%u, %s)\n", t, localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "[Critical %s]: %s (%s:%u, %s)\n", t, localMsg.constData(), context.file, context.line, context.function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "[Fatal %s]: %s (%s:%u, %s)\n", t, localMsg.constData(), context.file, context.line, context.function);
        abort();
    }
    fflush(stderr);
}

int main(int argc, char *argv[])
{
    //Root path
    char* exe_root = (char*)malloc(sizeof(char) * (strlen(argv[0])-7));
    memcpy(exe_root, &argv[0][0], strlen(argv[0])-7);

    //Set log file
    //Redirect stderr to write log
    //Redirect stdin to read log
    std::string logfile = std::string(exe_root)+"YTLOG.txt";

    if(std::fopen(logfile.c_str(), "a") == NULL) {
        fprintf(stdout, "Can not set log file.");
        return 1;
    }
    std::freopen(logfile.c_str(), "r",stdin);
    std::freopen(logfile.c_str(), "a",stderr);

    qInstallMessageHandler(myMessageOutput);
    QApplication a(argc, argv);
    MainWindow w(exe_root);
    w.show();
    return a.exec();
}
