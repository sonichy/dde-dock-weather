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
    QImage image;

signals:
    void requestUpdateGeometry() const;
    void requestContextMenu() const;

public slots:    

private:
    QSize sizeHint() const;
    void resizeEvent(QResizeEvent *e);
    void paintEvent(QPaintEvent *e);
    void mousePressEvent(QMouseEvent *e);    
    QSettings m_settings;    
};

#endif // WEATHERWIDGET_H
