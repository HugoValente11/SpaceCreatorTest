#include "delayedsignal.h"

#include <QTimer>

namespace utils {

DelayedSignal::DelayedSignal(QObject* parent)
    : QObject(parent)
{
    mTimer = new QTimer(this);
    mTimer->setSingleShot(true);
    connect(mTimer, SIGNAL(timeout()), SLOT(triggerNow()));
}

void DelayedSignal::triggerSignal() {
    mTimer->start();
}

void DelayedSignal::triggerNow() {
    emit triggered(parent());
}

}