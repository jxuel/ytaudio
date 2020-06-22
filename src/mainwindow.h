#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMediaPlayer>
#include <Python.h>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QThreadPool>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(char *exe_path, QWidget *parent = nullptr);
    ~MainWindow();
    QString playlist;
    void getLists();
Q_SIGNALS:
    void startWorker(int v);
    void dataReceived(QByteArray);

private Q_SLOTS:
    void on_addList_clicked();

    void on_playlistURL_textEdited(const QString &arg1);

    void on_playControl_clicked();

    void on_next_clicked();

    void on_prev_clicked();

    void newConnection();

    void disconnected();

    void readyRead();

private:
    QMediaPlayer *player;
    QMediaPlaylist *pList;
    QTcpServer* tcpServer;
    Ui::MainWindow *ui;
    char *exe_path;
    PyObject* pModule;
    PyObject* pGetLists;
    QHash<QTcpSocket*, QByteArray*> buffers;
    QHash<QTcpSocket*, qint32*> sizes;
    QThreadPool *threadPool;

};
#endif // MAINWINDOW_H
