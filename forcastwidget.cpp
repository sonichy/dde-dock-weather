#include "forcastwidget.h"
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonParseError>
#include <QFile>
#include <QStandardPaths>

// <中文，英文>
static const QMap<QString, QString> weatherMap {
    {"NA", "NA"},
    {"晴", "sunny"},
    {"阴", "cloudy"},
    {"雪", "snowy"},
    {"阵雨", "shower"},
    {"小雨", "drizzle"},
    {"中雨", "rain1"},
    {"大雨", "rain2"},
    {"多云", "atmosphere"},
};

ForcastWidget::ForcastWidget(QWidget *parent)
    : QWidget(parent),
      m_settings("deepin", "dde-dock-HTYWeather")
{
    setFixedWidth(250);
    QGridLayout *layout = new QGridLayout;
    for (int i=0; i<6; i++) {
        labelWImg[i] = new QLabel("");
        labelWImg[i]->setPixmap(QPixmap(":icon/NA.png").scaled(40, 40, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        labelWImg[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelWImg[i], i, 0);
        if (i==0) {
            labelTemp[i] = new QLabel("20°C");
            labelTemp[i]->setStyleSheet("color:white;font-size:20px;");
            labelDate[i] = new QLabel("城市");
            labelDate[i]->setStyleSheet("color:white;font-size:20px;");
        } else {
            labelTemp[i] = new QLabel("15~20°C");
            labelTemp[i]->setStyleSheet("color:white;font-size:12px;");
            labelDate[i] = new QLabel("1月1日 周一");
            labelDate[i]->setStyleSheet("color:white;font-size:12px;");
        }
        labelTemp[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelTemp[i],i,1);
        labelDate[i]->setAlignment(Qt::AlignCenter);
        layout->addWidget(labelDate[i],i,2);
    }
    setLayout(layout);
}

void ForcastWidget::updateWeather()
{
    QDateTime currentDateTime = QDateTime::currentDateTime();
    QString city = "", sw0 = "", wendu = "", stip = "", surl="";
    QString log = currentDateTime.toString("yyyy/MM/dd HH:mm:ss") + "\n";
    QNetworkAccessManager manager;
    QEventLoop loop;
    QNetworkReply *reply;

    QString setting_city = m_settings.value("city","").toString();
    if(setting_city != ""){
        city = setting_city;
    }else{
        // IP转城市名
        QString surl = "http://ip-api.com/json/?lang=zh-CN";
        QUrl url(surl);
        QNetworkAccessManager manager;
        QEventLoop loop;
        QNetworkReply *reply;
        reply = manager.get(QNetworkRequest(url));
        //请求结束并下载完成后，退出子事件循环
        QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        //开启子事件循环
        loop.exec();
        QByteArray BA = reply->readAll();
        qDebug() << surl ;
        qDebug() << BA;
        log += surl + "\n";
        log += BA + "\n";
        QJsonParseError JPE;
        QJsonDocument JD = QJsonDocument::fromJson(BA, &JPE);
        if(JPE.error == QJsonParseError::NoError) {
            if(JD.isObject()) {
                QJsonObject obj = JD.object();
                if(obj.contains("city")) {
                    QJsonValue JV_city = obj.take("city");
                    if(JV_city.isString()) {
                        city = JV_city.toString();
                    }
                }
            }
        }
    }

    if(city.contains("(")){
        city = city.right(city.length() - city.indexOf("(") -1);
        if(city.contains(" ")) city = city.left(city.indexOf(" "));
    }
    // 根据城市名取cityID
    surl = "http://hao.weidunewtab.com/tianqi/city.php?city=" + city;
    QUrl url(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QString cityID = reply->readAll();
    qDebug() << surl;
    qDebug() << cityID;
    log += surl + "\n";
    log += cityID + "\n";

    // 根据cityID取天气预报
    surl = "http://t.weather.sojson.com/api/weather/city/"+ cityID;
    url.setUrl(surl);
    reply = manager.get(QNetworkRequest(url));
    QObject::connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();
    QByteArray BA = reply->readAll();
    qDebug() << surl;
    //qDebug() << BA;
    log += surl + "\n";
    log += BA + "\n";
    QJsonParseError JPE;
    QJsonDocument JD = QJsonDocument::fromJson(BA, &JPE);
    //qDebug() << "QJsonParseError:" << JPE.errorString();
    if (JPE.error == QJsonParseError::NoError) {
        if (JD.isObject()) {
            QJsonObject obj = JD.object();
            city = obj.value("cityInfo").toObject().value("city").toString().replace("市","");
            labelDate[0]->setText(city);
            QString SUT = obj.value("cityInfo").toObject().value("updateTime").toString();
            //if (obj.contains("data")) {
            QJsonObject JO_data = JD.object().value("data").toObject();
            wendu = JO_data.value("wendu").toString() + "°C";
            labelTemp[0]->setText(wendu);
            QString shidu = JO_data.value("shidu").toString();
            QString pm25 = QString::number(JO_data.value("pm25").toInt());
            QString quality = JO_data.value("quality").toString();
            QString ganmao = JO_data.value("ganmao").toString();

            //if(JO_data.contains("forecast")){
            QPixmap pixmap0;
            QJsonArray JA_forecast = JO_data.value("forecast").toArray();
            for (int i=0; i<JA_forecast.size(); i++) {
                QString wtype = JA_forecast[i].toObject().value("type").toString();
                QString icon_path = ":/icon/" + weatherMap[wtype] + ".png";
                QPixmap pixmap(icon_path);
                if(i == 0){
                    pixmap0 = pixmap;
                    sw0 = wtype;
                    labelWImg[i]->setToolTip(wtype);
                    labelWImg[i]->setPixmap(pixmap.scaled(60,60));
                    labelWImg[i]->setAlignment(Qt::AlignCenter);
                }
                labelTemp[i+1]->setText(wtype + " " + JA_forecast[i].toObject().value("low").toString().replace("低温","").replace("℃","").replace(" ","") + " ~ " + JA_forecast[i].toObject().value("high").toString().replace("高温","").replace(" ",""));
                labelTemp[i+1]->setAlignment(Qt::AlignCenter);
                labelDate[i+1]->setText(JA_forecast[i].toObject().value("date").toString());
                labelDate[i+1]->setAlignment(Qt::AlignCenter);
                labelWImg[i+1]->setToolTip(wtype);
                labelWImg[i+1]->setPixmap(pixmap.scaled(40,40));
                labelWImg[i+1]->setAlignment(Qt::AlignCenter);
            }
            //}
            //}
            stip = city + "\n" + sw0 + "\n" + wendu + "\n湿度：" + shidu + "\nPM2.5：" + pm25 + "\n空气质量：" + quality +"\n" + ganmao + "\n更新：" + SUT;
            emit weatherNow(sw0, wendu, stip, pixmap0);
        }
    }

    // 写log
    QString path = QStandardPaths::standardLocations(QStandardPaths::CacheLocation).first() + "/HTYWeather.log";
    qDebug() << path;
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(log.toUtf8());
        file.close();
    }

}
