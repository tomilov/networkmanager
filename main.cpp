#include "networkmanager.hpp"

#include <QtCore>
#include <QtGui>
#include <QtQml>

#include <utility>

template< typename T >
QObject *
instance(QQmlEngine * engine, QJSEngine * scriptEngine)
{
    Q_UNUSED(scriptEngine);
    T * const t = ::new T{engine};
    t->setObjectName(QLatin1String(T::staticMetaObject.className()));
    return t;
}

static
void loadTranslations(QLoggingCategory const & loggingCategory = *QLoggingCategory::defaultCategory(),
                      QStringList translations = {},
                      QLocale locale = {QLocale::Russian, QLocale::Russia},
                      QString project = QStringLiteral(PROJECT_NAME))
{
    QLocale::setDefault(locale);
    translations.prepend(project);
    for (QString const & translation : translations) {
        QScopedPointer translator{::new QTranslator{qApp}};
        if (translator->load(locale, translation, ".", ":/translations")) {
            if (!QCoreApplication::installTranslator(translator.take())) {
                qCDebug(loggingCategory).noquote()
                        << QTranslator::tr("Unable to install translation for %1 locale from project %2")
                           .arg(locale.name(), translation);
            }
        } else {
            qCDebug(loggingCategory).noquote()
                    << QTranslator::tr("Unable to load translation for %1 locale from project %2")
                       .arg(locale.name(), translation);
        }
    }
}

int main(int argc, char * argv [])
{
    bool ok = false;

    QSettings::setDefaultFormat(QSettings::Format::IniFormat);

    QGuiApplication::setOrganizationName(ORGANIZATION_NAME);
    QGuiApplication::setOrganizationDomain(ORGANIZATION_DOMAIN);
    QGuiApplication::setApplicationName(PROJECT_NAME);
    QGuiApplication::setApplicationVersion(PROJECT_VERSION);
    QGuiApplication::setApplicationDisplayName(PROJECT_NAME);

    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QGuiApplication application{argc, argv};

    loadTranslations(networkManagerCategory());

    {
        QFont font = application.font();
        font.setPointSize(QSettings{}.value("fontSize", 24).toInt(&ok));
        Q_ASSERT(std::exchange(ok, false));
        application.setFont(font);
    }

    qmlRegisterType< NetworkManager >();
    qmlRegisterSingletonType< NetworkManagerSingleton >("NetworkManager", 1, 0, "NetworkManager", &instance< NetworkManagerSingleton >);

    QUrl source{R"(qrc:/qml/ui.qml)"};
    QQmlApplicationEngine engine;
    engine.load(source);

    return application.exec();
}
