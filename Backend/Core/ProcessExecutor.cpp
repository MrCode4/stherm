#include "ProcessExecutor.h"
#include <QEventLoop>
#include <QTimer>
#include <QUuid>
#include <QDebug>


int ProcessExecutor::waitLoop(QProcess* process, uint timeout) const
{
    if (timeout <= 0) {
        timeout = PROCESS_DEFAULT_TIMEOUT;
    }

    QEventLoop loop;
    // connect signal for handling stopWork
    connect(process, &QProcess::finished, &loop, [&loop]() {
        loop.exit();
    });

    // quit will exit with, same as exit(ChangeType::CurrentTemperature)
    QTimer::singleShot(timeout, &loop, [&loop, process](){
        if (process->state() != QProcess::NotRunning) {
            process->terminate();
        }
    });

    return loop.exec();
}

void ProcessExecutor::preExec(QProcess* process, const QString& command, const QStringList& args, const QString& logline)
{
    qInfo() << "STARTING: " + logline;
    mProcesses.append(process);
    process->setReadChannel(QProcess::StandardOutput);
    connect(process, &QProcess::started, this, [this, logline] () {
        qInfo() << "STARTED: " + logline;
    });
    process->start(command, args);
}

void ProcessExecutor::postExec(QProcess* process, ExitedCallback callback, const QString& logline)
{
    mProcesses.removeOne(process);
    qInfo() << "FINISHED: " + logline;
    if (process->exitCode() != 0 || process->exitStatus() != QProcess::NormalExit) {
        qWarning() << "ERROR: " + logline + " --> " << process->exitCode() << " / " << process->errorString();
    }
    if (callback) {
        callback(process);
    }
    emit finished(process->exitCode(), process->exitStatus());
}

void ProcessExecutor::execSync(const QString& command, const QStringList& args, ExitedCallback callback, uint timeout)
{
    QString logline = QUuid::createUuid().toString() + "# " + command + " " + args.join(' ');
    QProcess process;
    preExec(&process, command, args, logline);
    waitLoop(&process, timeout);
    postExec(&process, callback, logline);
}

void ProcessExecutor::execAsync(const QString& command, const QStringList& args, ExitedCallback callback, InitCallback init)
{
    QString logline = QUuid::createUuid().toString() + "# " + command + " " + args.join(' ');
    QProcess* process = new QProcess;
    if (init) {
        init(process);
    }
    preExec(process, command, args, logline);
    connect(process, &QProcess::finished, this, [this, process, callback, logline](int exitCode, QProcess::ExitStatus exitStatus) {
        postExec(process, callback, logline);
        delete process;
    }, Qt::SingleShotConnection);
}

void ProcessExecutor::kill()
{
    while (!mProcesses.isEmpty()) {
        mProcesses.takeFirst()->kill();
    }
}
