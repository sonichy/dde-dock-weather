#include "weatherplugin.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QAction>

WeatherPlugin::WeatherPlugin(QObject *parent)
    : QObject(parent),
      m_tipsLabel(new QLabel),
      m_refershTimer(new QTimer(this))
{    
    m_tipsLabel->setObjectName("weather");
    m_tipsLabel->setStyleSheet("color:white; padding:0px 3px;");

    m_refershTimer->setInterval(1800000);
    m_refershTimer->start();
    connect(m_refershTimer, &QTimer::timeout, this, &WeatherPlugin::updateWeather);

    m_centralWidget = new WeatherWidget;
    connect(m_centralWidget, &WeatherWidget::requestContextMenu, [this] { m_proxyInter->requestContextMenu(this, QString()); });
    connect(m_centralWidget, &WeatherWidget::requestUpdateGeometry, [this] { m_proxyInter->itemUpdate(this, QString()); });

    city="",stip="",temp="",sw="";

    forcastWidget = new DBlurEffectWidget;
    //forcastWidget = new QWidget;
    forcastWidget->resize(220,330);
    forcastWidget->setWindowFlags(Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint);
    //forcastWidget->setAttribute(Qt::WA_TranslucentBackground,true);
    forcastWidget->setMaskColor(DBlurEffectWidget::DarkColor);

    QAction *action_close = new QAction("关闭", this);
    connect(action_close,SIGNAL(triggered(bool)),forcastWidget,SLOT(hide()));
    forcastWidget->addAction(action_close);
    forcastWidget->setContextMenuPolicy(Qt::ActionsContextMenu);

    layout = new QGridLayout;
    for (int i=0; i<6; i++) {
        labelWImg[i] = new QLabel("");
        labelWImg[i]->setPixmap(QPixmap(":icon/clear.svg").scaled(40,40,Qt::KeepAspectRatio,Qt::SmoothTransformation));
        labelWImg[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelWImg[i],i,0);
        labelTemp[i] = new QLabel("15~20°C");
        labelTemp[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelTemp[i],i,1);
        if (i==0) {
            labelTemp[i]->setStyleSheet("color:white;font-size:20px;");
            labelDate[i] = new QLabel("城市");
            labelDate[i]->setStyleSheet("color:white;font-size:20px;");
        } else {
            labelTemp[i]->setStyleSheet("color:white;font-size:12px;");
            labelDate[i] = new QLabel("1月1日");
            labelDate[i]->setStyleSheet("color:white;font-size:12px;");
        }
        labelDate[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelDate[i],i,2);
    }
    forcastWidget->setLayout(layout);

    updateWeather();
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
    return -1;
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

const QString WeatherPlugin::itemCommand(const QString &itemKey)
{
    Q_UNUSED(itemKey);
    forcastWidget->move(QApplication::desktop()->width() - forcastWidget->width() - 5, QApplication::desktop()->height() - forcastWidget->height() -50);
    forcastWidget->show();
    forcastWidget->raise();
    return "";
}

const QString WeatherPlugin::itemContextMenu(const QString &itemKey)
{
    Q_UNUSED(itemKey);

    QList<QVariant> items;
    items.reserve(1);

    QMap<QString, QVariant> about;
    about["itemId"] = "about";
    about["itemText"] = "关于";
    about["isActive"] = true;
    items.push_back(about);

    QMap<QString, QVariant> forecast;
    forecast["itemId"] = "forecast";
    forecast["itemText"] = "预报";
    forecast["isActive"] = true;
    items.push_back(forecast);

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
    menuitems << "about" << "forecast";

    switch(menuitems.indexOf(menuId)){
    case 0:
        MBAbout();
        break;
    case 1:
        forcastWidget->move(QApplication::desktop()->width() - forcastWidget->width()- 5, QApplication::desktop()->height() - forcastWidget->height() - 50);
        forcastWidget->show();
        forcastWidget->raise();
        break;
    }
}

void WeatherPlugin::MBAbout()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "天气预报 4.0", "关于\n\n深度Linux系统上一款在任务栏显示天气的插件。\n作者：黄颖\nE-mail: sonichy@163.com\n源码：https://github.com/sonichy/WEATHER_DDE_DOCK\n致谢：\nlinux028@deepin.org");
    aboutMB.setIconPixmap(QPixmap(":/icon/clear.svg"));
    aboutMB.exec();
}

void WeatherPlugin::updateWeather()
{
    // IP转城市名
    QString surl = "http://int.dpool.sina.com.cn/iplookup/iplookup.php?format=json";
    QUrl url(surl);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray BA = reply->readAll();
    qDebug() << surl ;
    qDebug() << BA;
    QJsonParseError JPE;
    QJsonDocument JD = QJsonDocument::fromJson(BA, &JPE);
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isObject()) {
            QJsonObject JO = JD.object();
            if (JO.contains("city")) {
                QJsonValue JV = JO.take("city");
                if (JV.isString()) {
                    city = JV.toString();
                }
            }
        }
    }

    // 根据城市名取经纬度
    QString latitude="", longitude="";
    surl = "http://w.api.deepin.com/v1/location/" + city;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    BA = reply->readAll();
    qDebug() << surl ;
    qDebug() << BA;
    JD = QJsonDocument::fromJson(BA, &JPE);
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isArray()) {
            QJsonArray JA = JD.array();
            latitude = JA[0].toObject().value("latitude").toString();
            longitude = JA[0].toObject().value("longitude").toString();
        }
    }

    // 根据经纬度取天气预报
    surl = "http://w.api.deepin.com/v1/forecast/" + latitude + "/" + longitude;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    BA = reply->readAll();
    qDebug() << surl;
    qDebug() << BA;
    JD = QJsonDocument::fromJson(BA, &JPE);
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isArray()) {
            QJsonArray JA = JD.array();
            for (int i=0; i<JA.size(); i++) {
                QString weatherName = JA[i].toObject().value("name").toString();
                QString fileName = ":icon/" + weatherName.toLower() + ".svg";
                if (i == 0) {
                    QPixmap pixmap(fileName);
                    labelWImg[i]->setPixmap(pixmap.scaled(80,80,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                    m_centralWidget->pixmap = pixmap;
                    temp = QString::number(JA[i].toObject().value("temperatureMin").toInt()) + "°C";
                    labelTemp[i]->setText(temp);
                    m_centralWidget->temp = temp;
                    labelDate[i]->setText(city);
                    sw = translateWeather(weatherName);
                    m_centralWidget->sw = sw;
                    QDateTime currentDateTime = QDateTime::currentDateTime();
                    stip = city + "\n" + sw + "\n" + temp + "\n刷新：" + currentDateTime.toString("HH:mm:ss");
                    m_tipsLabel->setText(stip);
                } else {
                    labelWImg[i]->setPixmap(QPixmap(fileName).scaled(40,40,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                    labelTemp[i]->setText(QString::number(JA[i].toObject().value("temperatureMin").toInt()) + "~" + QString::number(JA[i].toObject().value("temperatureMax").toInt()) + "°C");
                    QDateTime date = QDateTime::fromTime_t(JA[i].toObject().value("date").toInt());
                    labelDate[i]->setText(date.toString("M月d日"));
                }
            }
        }
    }

}

QString WeatherPlugin::translateWeather(QString s)
{
    QString sc = "";
    if(s == "Atmosphere"){
        return "霾";
    } else if (s == "Clear") {
        return "晴";
    } else if (s == "Clouds") {
        return "多云";
    } else if (s == "Drizzle") {
        return "毛毛雨";
    } else if (s == "Rain") {
        return "雨";
    } else if (s == "Snow") {
        return "雪";
    } else if (s == "Thunderstorm") {
        return "雷雨";
    }
    return sc;
}
