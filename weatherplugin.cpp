#include "weatherplugin.h"
#include <QApplication>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>

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

    city="",cityID="",swtips="",temp="",sw1="";
    //label = (QLabel *)m_mainWidget;
    //label->setText("天气\n温度");
    //label->setStyleSheet("color:white;padding:0px;");
    //label->setAlignment(Qt::AlignCenter);
    //label->setFixedWidth(50);
    //m_tipsLabel->setStyleSheet("color:white; padding:5px;");

    //window
    window = new QWidget;
    window->setWindowTitle("中国天气预报");
    window->setFixedSize(500,240);
    window->setStyleSheet("QLabel{ color:white; }");
    //居中
    window->move((QApplication::desktop()->width() - window->width())/2, (QApplication::desktop()->height() - window->height())/2);
    // 移除最小化
    window->setWindowFlags((window->windowFlags() | Qt::CustomizeWindowHint) & ~Qt::WindowMinimizeButtonHint);
    // 不在任务栏显示
    window->setWindowFlags(Qt::Tool);
    // 隐藏标题栏
    //window->setWindowFlags(Qt::FramelessWindowHint);
    // 背景透明
    window->setAttribute(Qt::WA_TranslucentBackground, true);

    layout = new QGridLayout;
    labelCity = new QLabel("城市");
    labelCity->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelCity,0,0);
    labelTemp = new QLabel("温度\n?");
    labelTemp->setStyleSheet("font-size:40px;");
    labelTemp->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelTemp,0,1);
    labelSD = new QLabel("湿度\n?");
    labelSD->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelSD,0,2);
    labelWind = new QLabel("风向?\n风力?");
    labelWind->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelWind,0,3);
    labelPM = new QLabel("PM2.5\n?");
    labelPM->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelPM,0,4);
    labelAQI = new QLabel("空气质量指数\n?");
    labelAQI->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelAQI,0,5);
    labelRT = new QLabel("刷新\n?");
    labelRT->setAlignment(Qt::AlignCenter);
    layout->addWidget(labelRT,0,6);
    for(int i=1;i<8;i++){
        labelDate[i-1] = new QLabel("1月1日\n星期一");
        labelDate[i-1]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelDate[i-1],1,i-1);
        labelWImg[i-1] = new QLabel("");

        QImage image;
        image.load(":/images/0.png");
        labelWImg[i-1]->setPixmap(QPixmap::fromImage(image.scaled(50,50)));
        labelWImg[i-1]->setAlignment(Qt::AlignCenter);

        layout->addWidget(labelWImg[i-1],2,i-1);
        labelWeather[i-1] = new QLabel("晴\n15°C ~ 25°C\n北风1级");
        labelWeather[i-1]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelWeather[i-1],3,i-1);
    }
    labelComment = new QLabel;
    layout->addWidget(labelComment,4,0,1,7);
    window->setLayout(layout);
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
    window->move((QApplication::desktop()->width() - window->width())/2, (QApplication::desktop()->height() - window->height())/2);
    window->show();
    window->raise();
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
        window->move((QApplication::desktop()->width() - window->width())/2, (QApplication::desktop()->height() - window->height())/2);
        window->show();
        window->raise();
        break;
    }
}

void WeatherPlugin::MBAbout()
{
    QMessageBox aboutMB(QMessageBox::NoIcon, "天气预报 3.2", "关于\n\n深度Linux系统上一款在任务栏显示天气的插件。\n作者：黄颖\nE-mail: sonichy@163.com\n源码：https://github.com/sonichy/WEATHER_DDE_DOCK\n致谢：\nlinux028@deepin.org");
    aboutMB.setIconPixmap(QPixmap(":/images/0.png"));
    aboutMB.exec();
}

void WeatherPlugin::updateWeather()
{
    // IP地址转城市名
    QString surl = "http://int.dpool.sina.com.cn/iplookup/iplookup.php?format=json";
    QUrl url(surl);
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;
    reply = manager.get(QNetworkRequest(url));
    //请求结束并下载完成后，退出子事件循环
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    //开启子事件循环
    loop.exec();
    QString codeContent = reply->readAll();
    qDebug() << surl;
    qDebug() << codeContent;
    QJsonParseError json_error;
    QJsonDocument parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError) {
        if(parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("city")) {
                QJsonValue city_value = obj.take("city");
                if(city_value.isString()) {
                    city = city_value.toString();
                }
            }
        }
    }

    // 城市名转城市ID
    surl = "http://hao.weidunewtab.com/tianqi/city.php?city=" + city;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    cityID = reply->readAll();
    qDebug() << surl;
    qDebug()<< cityID;
    if (cityID == "") {
        labelComment->setText("错误：城市名返回城市ID为空");
    } else {
        bool ok;
        int dec = cityID.toInt(&ok, 10);
        if(!ok){
            labelComment->setText(reply->readAll());
        }
    }

    // 7天预报
    surl = "http://hao.weidunewtab.com/myapp/weather/data/index.php?cityID=" + cityID;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << surl;
    qDebug() << codeContent;
    parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError) {
        if(parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("weatherinfo")) {
                QJsonObject::iterator it;
                it = obj.find("weatherinfo");
                QJsonObject weatherinfoObj = it.value().toObject();
                city = weatherinfoObj.value("city").toString();
                labelCity->setText(city);
                sw1 = weatherinfoObj.value("weather1").toString();
                m_centralWidget->sw1 = sw1;
                QString sdate = weatherinfoObj.value("date_y").toString();
                QDate date;
                if (sdate.contains("年") && sdate.contains("月")) {
                    date = QDate::fromString(weatherinfoObj.value("date_y").toString(), "yyyy年M月d");
                } else {
                    date = QDate::currentDate();
                }
                for (int i=1; i<8; i++) {
                    labelDate[i-1]->setText(date.addDays(i-1).toString("M-d")+"\n"+date.addDays(i-1).toString("dddd"));
                    labelDate[i-1]->setAlignment(Qt::AlignCenter);

                    QImage image;
                    surl = ":/images/" + QString::number(weatherinfoObj.value("img"+QString::number(2*i-1)).toInt()) + ".png";
                    image.load(surl);
                    labelWImg[i-1]->setPixmap(QPixmap::fromImage(image.scaled(50,50)));
                    labelWImg[i-1]->setAlignment(Qt::AlignCenter);
                    if (i==1) m_centralWidget->image = image;

                    labelWeather[i-1]->setText(weatherinfoObj.value("weather" + QString::number(i)).toString() + "\n" + weatherinfoObj.value("temp"+QString::number(i)).toString() + "\n" + weatherinfoObj.value("wind"+QString::number(i)).toString());
                    labelWeather[i-1]->setAlignment(Qt::AlignCenter);
                }
                m_centralWidget->update();
            }
        }
    }

    // 实时天气
    surl = "http://hao.weidunewtab.com/myapp/weather/data/indexInTime.php?cityID=" + cityID;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    codeContent = reply->readAll();
    qDebug() << surl;
    qDebug() << codeContent;
    parse_doucment = QJsonDocument::fromJson(codeContent.toLatin1(), &json_error);
    if(json_error.error == QJsonParseError::NoError) {
        if(parse_doucment.isObject()) {
            QJsonObject obj = parse_doucment.object();
            if(obj.contains("weatherinfo")) {
                const QDateTime currentDateTime = QDateTime::currentDateTime();
                QJsonObject::iterator it;
                it = obj.find("weatherinfo");
                QJsonObject weatherinfoObj = it.value().toObject();
                swtips = weatherinfoObj.value("city").toString() + "\n" + sw1 + "\n" + weatherinfoObj.value("temp").toString() + "°C\n湿度：" + weatherinfoObj.value("SD").toString() + "\n" + weatherinfoObj.value("WD").toString() + weatherinfoObj.value("WS").toString() + "\nPM2.5：" + weatherinfoObj.value("pm25").toString() + "\n空气质量指数："+ QString::number(weatherinfoObj.value("aqiLevel").toInt()) + "\n刷新：" + currentDateTime.toString("HH:mm:ss");
                m_tipsLabel->setText(swtips);
                temp = weatherinfoObj.value("temp").toString() + "°C";
                m_centralWidget->temp = temp;
                labelTemp->setText(weatherinfoObj.value("temp").toString() + "°C");
                labelSD->setText("湿度\n" + weatherinfoObj.value("SD").toString());
                labelWind->setText(weatherinfoObj.value("WD").toString() + "\n" + weatherinfoObj.value("WS").toString());
                labelPM->setText("PM2.5\n" + weatherinfoObj.value("pm25").toString());
                labelAQI->setText("空气质量指数\n" + QString::number(weatherinfoObj.value("aqiLevel").toInt()));
                labelRT->setText("刷新\n" + currentDateTime.toString("HH:mm:ss"));
            }
        }
    }

}
