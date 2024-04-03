#include "ScreenSaverManager.h"
#include "LogHelper.h"

#include <QMouseEvent>

/*!
 * \brief ScreenSaverManager::sInstance Initialize it to null
 */
ScreenSaverManager* ScreenSaverManager::sInstance = nullptr;

ScreenSaverManager* ScreenSaverManager::instance()
{
    return sInstance;
}

ScreenSaverManager::ScreenSaverManager(QObject *parent)
    : QObject{parent}
    , mApplication { QCoreApplication::instance() }
    , mState { ScreenSaverManager::State::Disabled }
{
    //! Set sInstance to this
    if (!sInstance) {
        sInstance = this;
    }

    //! By default screen saver timeout is 20000
    mScreenSaverTimer.setInterval(20000);

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
    TRACE <<mState << newState;
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
        TRACE << mScreenSaverTimer.isActive();
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
    TRACE << mSetAppActive;

    if (!mSetAppActive)
        return;

    setState(State::Running); //! This will start timer and event filter

    //! Add event filter
    if (mApplication) {
        mApplication->installEventFilter(this);
    }
}

void ScreenSaverManager::restart()
{
    if (mState == State::Running) {
        mScreenSaverTimer.stop();
        mScreenSaverTimer.start();
    } else if (mState != State::Disabled){
        setActive();
    }
}

void ScreenSaverManager::setAppActive(bool setAppActive)
{
    TRACE << setAppActive;
    mSetAppActive = setAppActive;
}

bool ScreenSaverManager::eventFilter(QObject* watched, QEvent* event)
{
    switch (event->type()) {
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
