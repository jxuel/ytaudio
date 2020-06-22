#ifndef URLWORKER_H
#define URLWORKER_H
#include <QObject>
#include <Python.h>
#include <QRunnable>




class UrlWorker : public QObject
{
    Q_OBJECT
public:
    UrlWorker(PyObject* pFunc, std::string playlist);
    ~UrlWorker();
public Q_SLOTS:
      void doWork();
Q_SIGNALS:
      void sglFinished();
      void sglStarted();
private:
      PyObject* pFunc;
      std::string playlist;
      int numUrls;
};

#endif // URLWORKER_H
