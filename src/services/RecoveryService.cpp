#include "services/RecoveryService.h"

#include "services/FileService.h"

#include <QCryptographicHash>
#include <QDir>
#include <QFile>
#include <QStandardPaths>

QString RecoveryService::recoveryDirectory()
{
    const QByteArray overrideDir = qgetenv("SLATEMARK_RECOVERY_DIR");
    if (!overrideDir.isEmpty()) {
        const QString dir = QString::fromLocal8Bit(overrideDir);
        QDir().mkpath(dir);
        return dir;
    }
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + QStringLiteral("/recovery");
    QDir().mkpath(dir);
    return dir;
}

QString RecoveryService::recoveryPathFor(const QString& documentKey)
{
    const QByteArray hash = QCryptographicHash::hash(documentKey.toUtf8(), QCryptographicHash::Sha1).toHex();
    return recoveryDirectory() + '/' + QString::fromLatin1(hash) + QStringLiteral(".md.recovery");
}

bool RecoveryService::saveRecovery(const QString& documentKey, const QString& text, QString* error)
{
    const FileResult result = FileService::writeTextFile(recoveryPathFor(documentKey), text);
    if (!result.ok && error) {
        *error = result.error;
    }
    return result.ok;
}

QString RecoveryService::loadRecovery(const QString& documentKey)
{
    const FileResult result = FileService::readTextFile(recoveryPathFor(documentKey));
    return result.ok ? result.text : QString();
}

void RecoveryService::removeRecovery(const QString& documentKey)
{
    QFile::remove(recoveryPathFor(documentKey));
}
