#pragma once

#include <QtDebug>
#include <QObject>
#include <QProcess>
#include <QList>

/*! ***********************************************************************************************
 * Base class to manage external command/process execution.
 * ************************************************************************************************/

#define PROCESS_DEFAULT_TIMEOUT 500

class ProcessExecutor : public QObject {
    Q_OBJECT

public:
    // do not delete the pointer! its owned by the main caller
    using InitCallback = std::function<void (QProcess* process)>;
    // do not delete the pointer! its owned by the main caller
    using ExitedCallback = std::function<void (QProcess* process)>;
    explicit ProcessExecutor(QObject* parent = nullptr) : QObject(parent) {}

signals:
    void finished(int exitCode, QProcess::ExitStatus exitStatus);

public:
    void execSync(const QString& command, const QStringList& args, ExitedCallback callback, uint timeout);
    void execAsync(const QString& command, const QStringList& args, ExitedCallback callback, InitCallback init = nullptr);
    void kill();

private:
    int waitLoop(QProcess* process, uint timeout) const;
    void preExec(QProcess* process, const QString& command, const QStringList& args, const QString& cmdline);
    void postExec(QProcess* process, ExitedCallback callback, const QString& logline);

private:
    QList<QProcess*> mProcesses;
};
