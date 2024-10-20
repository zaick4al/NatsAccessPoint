#include <QCoreApplication>
#include "src/natsservice.h"

int main(int argc, char *argv[])
{
    NatsService service{argc, argv};
    QCoreApplication::setOrganizationName("Inbis");
    QCoreApplication::setApplicationName(QStringLiteral("NatsAccessPoint"));
    QCoreApplication::setApplicationVersion(QStringLiteral("0.1.0"));
    const auto appDir = QFileInfo{QString::fromUtf8(argv[0])}.dir();
    QString pluginPath = QString(appDir.absolutePath().toUtf8())+"/plugins";
    qputenv("QT_PLUGIN_PATH", pluginPath.toUtf8());
    return service.exec();
}
