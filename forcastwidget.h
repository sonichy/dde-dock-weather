#ifndef FORCASTWIDGET_H
#define FORCASTWIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>

class ForcastWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ForcastWidget(QWidget *parent = 0);

private:
    QVBoxLayout *m_centralLayout;
    QWidget *m_centralWidget;
    QLabel *labelCity, *labelRT, *labelDate[6], *labelWImg[6], *labelTemp[6];
    QString translateWeather(QString s);

public slots:
    void updateWeather();

signals:
    void weatherNow(QString weather, QString temp, QString stip, QPixmap pixmap);

};

#endif // FORCASTWIDGET_H
