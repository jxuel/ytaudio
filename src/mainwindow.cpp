#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include "urlworker.h"
#include <QMediaPlaylist>
#include <QThread>
#include <Python.h>

MainWindow::MainWindow(char * exe_path, QWidget *parent)
    : QMainWindow(parent)
    , playlist("")
    , player(new QMediaPlayer(this))
    , pList (new QMediaPlaylist())
    , ui(new Ui::MainWindow)
    , exe_path(exe_path)
    , threadPool(new QThreadPool())
{

    qDebug() << "MainWindow start init" <<endl;
    ui->setupUi(this);
    this->setWindowTitle("YTAudio");
    this->player->setPlaylist(this->pList);

    tcpServer = new QTcpServer(this);
    if(!tcpServer->listen(QHostAddress::LocalHost, 9342)) {
        qDebug() << "Server could not start";
    }  else {
        qDebug() << "Server started";
    }
    //Listen new connection
    QObject::connect(tcpServer, SIGNAL(newConnection()),this, SLOT(newConnection()));
    //Close server
    QObject::connect(qApp, &QApplication::aboutToQuit, this, [&]{
        tcpServer->close();
    });


    //Initialize python module
    Py_Initialize();
    if (!PyEval_ThreadsInitialized()) {
        PyEval_InitThreads();
        qDebug() << "PY thread reinitialized";
    }
    qDebug() << "Initialized python interpreter" <<endl;
    std::string path(this->exe_path);
    PyRun_SimpleString("import sys");
    std::string addPath = "sys.path.append('"+ path+"')";
    PyRun_SimpleString(addPath.c_str());
    this->pModule = PyImport_ImportModule("geturls");
    if(this->pModule == NULL )
        qDebug() << "Cannot load module geturls"<<endl;
    else
        qDebug() << "Loaded module geturls" <<endl;
    this->pGetLists = PyObject_GetAttrString(pModule, "getlists");

    //Release thread lock after py initialization
    PyThreadState *threadState = PyEval_SaveThread();


    // Status bar connect
    QObject::connect(this->player, &QMediaPlayer::positionChanged, ui->progressBar, &QProgressBar::setValue);
    QObject::connect(this->player, &QMediaPlayer::durationChanged, ui->progressBar, &QProgressBar::setMaximum);
    // Volume connect
    QObject::connect(ui->volume, &QSlider::valueChanged, this->player, &QMediaPlayer::setVolume);
    // Status control
    QObject::connect(this->player, &QMediaPlayer::stateChanged, [&](QMediaPlayer::State state){
        if (state == QMediaPlayer::State::PlayingState) {
            ui->playControl->setText("Pause");
        }
        else if (state == QMediaPlayer::State::PausedState)
            this->ui->playControl->setText("Play");
        else
            this->ui->playControl->setText("Play");
    });

    //Load entire log
    QObject::connect(ui->display, &QTabWidget::currentChanged, [&](int indx){
        if (indx == 1 && feof(stdin) == 0) {
            fseek(stdin, 0, SEEK_END);
            long fsize = ftell(stdin);
            fseek(stdin, 0, SEEK_SET);

            char *str = (char*)malloc(fsize + 1);
            fread(str, 1, fsize, stdin);
            fgetc(stdin);
            ui->logBrowser->insertPlainText(str);
        }
    });
    //Playlist info changed
    QObject::connect(this->pList, &QMediaPlaylist::mediaChanged, this, [&](){
        QString str = QString::number(this->pList->currentIndex()+1)+ '/' + QString::number(this->pList->mediaCount());
        this->ui->listNumber->setText(str);
    });
    QObject::connect(this->pList, &QMediaPlaylist::mediaRemoved, this, [&](){
        QString str = QString::number(this->pList->currentIndex()+1)+ '/' + QString::number(this->pList->mediaCount());
        this->ui->listNumber->setText(str);
    });
    QObject::connect(this->pList, &QMediaPlaylist::mediaInserted, this, [&](){
        QString str = QString::number(this->pList->currentIndex()+1)+ '/' + QString::number(this->pList->mediaCount());
        this->ui->listNumber->setText(str);
    });

    //Player error
    QObject::connect(this->player, QOverload<QMediaPlayer::Error>::of(&QMediaPlayer::error),[&](QMediaPlayer::Error error){
        switch(error) {
        case QMediaPlayer::NoError:
            break;
        case QMediaPlayer::ResourceError:
            qDebug() << "ResourceError" << endl;
            break;
        case QMediaPlayer::FormatError:
            qDebug() << "FormatError" << endl;
            break;
        case QMediaPlayer::NetworkError:
            qDebug() << "NetworkError" << endl;

            break;
        case QMediaPlayer::AccessDeniedError:
            qDebug() << "AccessDeniedError" << endl;

            break;
        case QMediaPlayer::ServiceMissingError:
            qDebug() << "ServiceMissingError" << endl;

            break;
        default:
            break;
        }
    });

    //Application quit process
    QObject::connect(qApp, SIGNAL(aboutToQuit()), this->player, SLOT(stop));
    qDebug() << "MainWindow Init Successfully" <<endl;
}


MainWindow::~MainWindow()
{
    delete ui;
    delete player;

    qDebug() << "MainWindow Closed" <<endl;
}

// Add a url
void MainWindow::on_addList_clicked()
{
    qDebug() << "Set playlist url: " << this->playlist << endl;
    this->getLists();
}

// Store current url in the textEditor
void MainWindow::on_playlistURL_textEdited(const QString &arg1)
{
    this->playlist = arg1;
}

// Player Controlller
void MainWindow::on_playControl_clicked()
{
    if (this->player->state() == QMediaPlayer::State::PlayingState) {
        this->player->pause();
    } else {
        this->player->play();
    }
}
// Play next
void MainWindow::on_next_clicked()
{
    this->pList->next();
    QString str = QString::number(this->pList->currentIndex()+1)+ '/' + QString::number(this->pList->mediaCount());
    this->ui->listNumber->setText(str);
}

// Play prev
void MainWindow::on_prev_clicked()
{
    this->pList->previous();
    QString str = QString::number(this->pList->currentIndex()+1)+ '/' + QString::number(this->pList->mediaCount());
    this->ui->listNumber->setText(str);
}


// New Connection
void MainWindow::newConnection() {
    qDebug() << "New connection" << endl;
    while (this->tcpServer->hasPendingConnections()) {
            QTcpSocket *socket = this->tcpServer->nextPendingConnection();
            connect(socket, SIGNAL(readyRead()), SLOT(readyRead()));
            connect(socket, SIGNAL(disconnected()), SLOT(disconnected()));
            QByteArray *buffer = new QByteArray();
            qint32 *s = new qint32(0);
            buffers.insert(socket, buffer);
            sizes.insert(socket, s);
        }
}

// Disconnect
void MainWindow::disconnected() {
    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
    QByteArray *buffer = buffers.value(socket);
    qint32 *s = sizes.value(socket);
    socket->deleteLater();
    delete buffer;
    delete s;
}

// Read Data
void MainWindow::readyRead()
{
    QTcpSocket* conn = qobject_cast<QTcpSocket*>(sender());
    QStringList list;
    QRegularExpression reUrl("^https?://");
    QRegularExpression reInit("^[INIT]");
    while (conn->size()) {
        QString data = QString(conn->readAll());
        std::string msg="";
        if (reUrl.match(data).hasMatch()) {
            qDebug()<< data << endl;
            this->pList->addMedia(QUrl(data));
            if (this->player->state() == QMediaPlayer::State::StoppedState) {
                this->player->play();
            }
        }
        msg = std::to_string(data.length());
        conn->write(msg.c_str());
    }
}

void MainWindow::getLists() {
    UrlWorker* worker = new UrlWorker(this->pGetLists, this->playlist.toStdString());

    QThread* thread = new QThread(0);
    worker->moveToThread(thread);
   // Start worker
    QObject::connect(thread, SIGNAL(started()), worker, SLOT(doWork()));
    //Clear thread and object
    QObject::connect(worker, SIGNAL(sglFinished()), thread, SLOT(quit()));
    QObject::connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    QObject::connect(qApp, SIGNAL(aboutToQuit()), thread, SLOT(deleteLater()));
    thread->start();
}
