#include "forcastwidget.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <QStandardPaths>

ForcastWidget::ForcastWidget(QWidget *parent)
    : QWidget(parent)
{
    setFixedWidth(250);

    QGridLayout *layout = new QGridLayout;
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
    setLayout(layout);
}

void ForcastWidget::updateWeather()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString city = "", sw = "", temp = "", stip = "", latitude = "", longitude = "";
    QString log = currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + "\n";

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
    log += surl + "\n";
    log += BA + "\n";
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
    surl = "http://w.api.deepin.com/v1/location/" + city;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    BA = reply->readAll();
    qDebug() << surl;
    qDebug() << BA;
    log += surl + "\n";
    log += BA + "\n";
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
    log += surl + "\n";
    log += BA;
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
                    temp = QString::number(JA[i].toObject().value("temperatureMin").toInt()) + "°C";
                    labelTemp[i]->setText(temp);
                    labelDate[i]->setText(city);
                    sw = translateWeather(weatherName);                    
                    stip = city + "\n" + sw + "\n" + temp + "\n刷新：" + currentDateTime.toString("HH:mm:ss");                    
                    emit weatherNow(sw, temp, stip, pixmap);
                } else {
                    labelWImg[i]->setPixmap(QPixmap(fileName).scaled(40,40,Qt::KeepAspectRatio,Qt::SmoothTransformation));
                    labelTemp[i]->setText(QString::number(JA[i].toObject().value("temperatureMin").toInt()) + "~" + QString::number(JA[i].toObject().value("temperatureMax").toInt()) + "°C");
                    QDateTime date = QDateTime::fromTime_t(JA[i].toObject().value("date").toInt());
                    labelDate[i]->setText(date.toString("M月d日 ddd"));
                }
            }
        }
    }

    // 写log
    QString path = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first() + "/weather.log";
    qDebug() << path;
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(log.toUtf8());
        file.close();
    }

}

QString ForcastWidget::translateWeather(QString s)
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
