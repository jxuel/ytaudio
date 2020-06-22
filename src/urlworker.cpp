#include "urlworker.h"
#include <QDebug>


UrlWorker::UrlWorker(PyObject* pF, std::string str)
    :pFunc(pF)
    ,playlist(str)
{

}
UrlWorker::~UrlWorker() {
}

void UrlWorker::doWork() {
    PyGILState_STATE gstate;
    gstate = PyGILState_Ensure();
    Q_EMIT sglFinished();
    qDebug() << "New thread started "<< endl;
    PyObject* pArgs = PyTuple_New(1);
    PyTuple_SetItem(pArgs, 0, Py_BuildValue("s", this->playlist.c_str()));
    PyObject* pReturn = NULL;
    pReturn = PyEval_CallObject(this->pFunc, pArgs);
    int count, total;
    PyArg_ParseTuple(pReturn, "i|i", &count, &total);
    PyGILState_Release(gstate);
    qDebug() << "Thread got: " << count << " Total in list: "<< total << endl;
    Q_EMIT sglFinished();
    delete this;
}


