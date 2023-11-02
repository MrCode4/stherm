#pragma once

#include <QObject>
#include <QQmlEngine>
#include <QCoreApplication>
#include <QTimer>

class ScreenSaverManager : public QObject
{
    Q_OBJECT

    Q_PROPERTY(State    state       READ getState NOTIFY stateChanged FINAL)
    Q_PROPERTY(int      screenSaverTimeout READ getScreenSaverTimeout WRITE setScreenSaverTimeout NOTIFY screenSaverTimeoutChanged FINAL)

    QML_ELEMENT
    QML_SINGLETON

public:
    /* Enums
     * ****************************************************************************************/
    /*!
     * \brief The State enum holds whether screen saver timer is inactive, active (timer is running
     * but not timed out yet) or timeout (screen saver should be shown). By default it is active and
     * screen saver timer is running
     */
    enum class State : uchar {
        Disabled, //! This state can be used to stop screen saver timer regardless of any event,
                  //! it can only be set from outside and will not be set to this internally.
                  //! Application events are not filtered (handled)
        Paused,   //! Timer is paused but handles (filters) application events
        Running,
        Timeout
    };
    Q_ENUM(State);

public:
    explicit ScreenSaverManager(QObject *parent = nullptr);

    /* Public methods
     * ****************************************************************************************/
    State               getState() const;
    void                setState(const State& newState);

    int                 getScreenSaverTimeout() const;
    void                setScreenSaverTimeout(int newScreenSaverTimeout);

    Q_INVOKABLE void    setInactive();
    Q_INVOKABLE void    setActive();

    /* Protected/private methods
     * ****************************************************************************************/
protected:
    virtual bool    eventFilter(QObject* watched, QEvent* event) override;

signals:
    void applicationChanged();
    void stateChanged();
    void screenSaverTimeoutChanged();

private:
    /*!
     * \brief mApplication Reference to the \a\b QCoreApplication instance of this application
     */
    QCoreApplication*    mApplication;

    /*!
     * \brief mVisible Whether screen saver should be visible or not
     */
    State               mState;

    /*!
     * \brief mScreenSaverTmr Timer for screen saver
     */
    QTimer              mScreenSaverTimer;

    /*!
     * \brief mAutoRestartOnPress If \a true \ref ScreenSaverManager will set state to
     * \ref State::Running automatically when a press event is received in \ref State::Timeout
     * state. If false state should be set back to \ref State::Running when appropriate.
     *
     * \details This is useful when press events needs to be delivered to a screen saver view.
     */
    bool                mAutoRestartOnPress = true;
};
