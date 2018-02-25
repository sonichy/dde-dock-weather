#ifndef WEATHERWIDGET_H
#define WEATHERWIDGET_H

#include <QWidget>
#include <QSettings>

class WeatherWidget : public QWidget
{
    Q_OBJECT

public:
    explicit WeatherWidget(QWidget *parent = 0);
    bool enabled();
    void setEnabled(const bool b);
    QString text;

signals:
    void requestUpdateGeometry() const;
    void requestContextMenu() const;

public slots:    

private:
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);
    QPixmap m_cachedIcon;
    QString m_cachedTime;
    QSettings m_settings;
    bool m_24HourFormat;
};

#endif // WEATHERWIDGET_H
