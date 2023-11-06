#include "ScreenSaverManager.h"

#include <QMouseEvent>

ScreenSaverManager::ScreenSaverManager(QObject *parent)
    : QObject{parent}
    , mApplication { QCoreApplication::instance() }
    , mState { ScreenSaverManager::State::Disabled }
{
    //! By default screen saver timeout is 20000
    mScreenSaverTimer.setInterval(20000);
    setActive();

    connect(&mScreenSaverTimer, &QTimer::timeout, this, [&]() {
        //! Set state to time out and stop timer
        setState(ScreenSaverManager::State::Timeout);
        mScreenSaverTimer.stop();
    });
}

ScreenSaverManager::State ScreenSaverManager::getState() const
{
    return mState;
}

void ScreenSaverManager::setState(const State& newState)
{
    if (mState == newState) {
        return;
    }

    mState = newState;
    emit stateChanged();

    switch (mState) {
    case State::Disabled:
        if (mApplication) {
            mApplication->removeEventFilter(this);
        }

        //! Stop timer.
        if (mScreenSaverTimer.isActive()) {
            mScreenSaverTimer.stop();
        }
    case State::Paused:
    case State::Timeout:
        //! Stop timer.
        if (mScreenSaverTimer.isActive()) {
            mScreenSaverTimer.stop();
        }
        break;
    case State::Running:
        if (!mScreenSaverTimer.isActive()) {
            mScreenSaverTimer.start();
        }
        break;
    }
}

int ScreenSaverManager::getScreenSaverTimeout() const
{
    return mScreenSaverTimer.interval();
}

void ScreenSaverManager::setScreenSaverTimeout(int newScreenSaverTimeout)
{
    if (mScreenSaverTimer.interval() == newScreenSaverTimeout) {
        return;
    }

    mScreenSaverTimer.setInterval(newScreenSaverTimeout);
    emit screenSaverTimeoutChanged();

    //! Restart timer
    if (mScreenSaverTimer.isActive()) {
        mScreenSaverTimer.stop();
        mScreenSaverTimer.start();
    }
}

void ScreenSaverManager::setInactive()
{
    setState(State::Disabled); //! This will stop timer and disable event filter.
}

void ScreenSaverManager::setActive()
{
    setState(State::Running); //! This will start timer and event filter

    //! Add event filter
    if (mApplication) {
        mApplication->installEventFilter(this);
    }
}

bool ScreenSaverManager::eventFilter(QObject* watched, QEvent* event)
{
    switch(event->type()) {
    case QEvent::TouchBegin:
    case QEvent::MouseButtonPress:
    case QEvent::KeyPress:
        if (mState == ScreenSaverManager::State::Running) {
            //! Set State to State::Paused if it's State::Running until a corresponding release
            setState(ScreenSaverManager::State::Paused);
        } else if (mState == ScreenSaverManager::State::Timeout) {
            //! Set it to running again
            setState(ScreenSaverManager::State::Running);

            //! Also return true so it's not propagated to lower layers
            //! Note that event itself should be accepted too, otherwise although press events won't
            //! be delivered to lower levels, drag events will be.
            event->setAccepted(true);
            return true;
        }
        break;
    case QEvent::MouseButtonRelease:
    case QEvent::KeyRelease:
    case QEvent::TouchCancel:
    case QEvent::TouchEnd:
        if (mState == ScreenSaverManager::State::Paused) {
            //! Set state to State::Running since this is pasued due to a corresponding press
            setState(ScreenSaverManager::State::Running);
        }
        break;
    case QEvent::ContextMenu:
    case QEvent::InputMethod:
        if (mState == ScreenSaverManager::State::Running) {
            //! Restart timer
            mScreenSaverTimer.stop();
            mScreenSaverTimer.start();
        }
        break;
    default:
        break;
    }

    //! Events should not be set as accepted (Except press event in State::Timeout)
    return false;
}
