#include "weatherplugin.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

WeatherPlugin::WeatherPlugin(QObject *parent)
    : QObject(parent),
      m_tipsLabel(new QLabel),
      m_refershTimer(new QTimer(this)),
      m_settings("deepin", "dde-dock-weather")
{    
    m_tipsLabel->setObjectName("weather");
    m_tipsLabel->setStyleSheet("color:white; padding:0px 3px;");

    m_centralWidget = new WeatherWidget;
    connect(m_centralWidget, &WeatherWidget::requestContextMenu, [this] { m_proxyInter->requestContextMenu(this, QString()); });
    connect(m_centralWidget, &WeatherWidget::requestUpdateGeometry, [this] { m_proxyInter->itemUpdate(this, QString()); });

    forcastApplet = new ForcastWidget;
    forcastApplet->setObjectName("forcast");
    forcastApplet->setVisible(false);
    connect(forcastApplet, SIGNAL(weatherNow(QString,QString,QString,QPixmap)), this, SLOT(weatherNow(QString,QString,QString,QPixmap)));
    forcastApplet->updateWeather();

    m_refershTimer->setInterval(1800000);
    m_refershTimer->start();
    connect(m_refershTimer, &QTimer::timeout, forcastApplet, &ForcastWidget::updateWeather);

}

const QString WeatherPlugin::pluginName() const
{
    return "weather";
}

const QString WeatherPlugin::pluginDisplayName() const
{
    return "天气";
}

void WeatherPlugin::init(PluginProxyInterface *proxyInter)
{
    m_proxyInter = proxyInter;
    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, QString());
}

void WeatherPlugin::pluginStateSwitched()
{
    m_centralWidget->setEnabled(!m_centralWidget->enabled());
    if (m_centralWidget->enabled())
        m_proxyInter->itemAdded(this, QString());
    else
        m_proxyInter->itemRemoved(this, QString());
}

bool WeatherPlugin::pluginIsDisable()
{
    return !m_centralWidget->enabled();
}

int WeatherPlugin::itemSortKey(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    return m_settings.value(key, 0).toInt();
}

void WeatherPlugin::setSortKey(const QString &itemKey, const int order)
{
    Q_UNUSED(itemKey);

    const QString key = QString("pos_%1").arg(displayMode());
    m_settings.setValue(key, order);
}

QWidget *WeatherPlugin::itemWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_centralWidget;
}

QWidget *WeatherPlugin::itemTipsWidget(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return m_tipsLabel;
}

QWidget *WeatherPlugin::itemPopupApplet(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    return forcastApplet;
}

const QString WeatherPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(0);

    QMap<QString, QVariant> about;
    about["itemId"] = "about";
    about["itemText"] = "关于";
    about["isActive"] = true;
    items.push_back(about);

    QMap<QString, QVariant> satalite;
    satalite["itemId"] = "satalite";
    satalite["itemText"] = "卫星云图";
    satalite["isActive"] = true;
    items.push_back(satalite);

    QMap<QString, QVariant> menu;
    menu["items"] = items;
    menu["checkableMenu"] = false;
    menu["singleCheck"] = false;
    return QJsonDocument::fromVariant(menu).toJson();
}

void WeatherPlugin::invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked)
{
    Q_UNUSED(itemKey)
    Q_UNUSED(checked)

    QStringList menuitems;
    menuitems << "about" << "satalite";

    switch(menuitems.indexOf(menuId)){
    case 0:
        MBAbout();
        break;
    case 1:
        showSatalite();
        break;
    }
}

void WeatherPlugin::MBAbout()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "天气预报 4.4", "关于\n\n深度Linux系统上一款在任务栏显示天气的插件。\n作者：黄颖\nE-mail: sonichy@163.com\n源码：https://github.com/sonichy/WEATHER_DDE_DOCK\n致谢：linux028@deepin.org");
    aboutMB.setIconPixmap(QPixmap(":/icon/clear.svg"));
    aboutMB.exec();
}

void WeatherPlugin::weatherNow(QString weather, QString temp, QString stip, QPixmap pixmap)
{
    m_centralWidget->sw = weather;
    m_centralWidget->temp = temp;
    m_centralWidget->pixmap = pixmap;
    m_tipsLabel->setText(stip);
}

void WeatherPlugin::showSatalite()
{
    QLabel *label = new QLabel;
    label->setWindowTitle("卫星云图");
    label->setWindowFlags(Qt::Tool);
    QString surl = "http://61.187.56.156/pic/zuixinyt/zuixinhw.png";
    QUrl url(surl);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray BA = reply->readAll();
    QPixmap pixmap;
    pixmap.loadFromData(BA);
    label->resize(pixmap.size());
    label->move((QApplication::desktop()->width() - label->width())/2, (QApplication::desktop()->height() - label->height())/2);
    label->setPixmap(pixmap);
    label->show();
}
