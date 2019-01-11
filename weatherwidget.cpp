#include "dde-dock/constants.h"
#include "weatherwidget.h"
#include <QApplication>
#include <QPainter>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

WeatherWidget::WeatherWidget(QWidget *parent)
    : QWidget(parent),
    m_settings("deepin", "dde-dock-HTYWeather")
{
    sw = "Weather";
    temp = "Temp";
    pixmap = QPixmap(":icon/na.png");
}

bool WeatherWidget::enabled()
{
    return m_settings.value(PLUGIN_STATE_KEY, true).toBool();
}

void WeatherWidget::setEnabled(const bool b)
{
    m_settings.setValue(PLUGIN_STATE_KEY, b);
}

QSize WeatherWidget::sizeHint() const
{
    QFontMetrics FM(qApp->font());
    QSize size;
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();
    if (displayMode == Dock::Efficient) {
        if(FM.boundingRect(sw).width() >= FM.boundingRect(temp).width()){
            size = FM.boundingRect(sw).size() + QSize(10,FM.boundingRect(sw).height());
        }else{
            size = FM.boundingRect(temp).size() + QSize(10,FM.boundingRect(temp).height());
        }
    }else{
        size = QPixmap(":icon/na.png").size();
    }
    return size;
}

void WeatherWidget::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
}

void WeatherWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);
    const Dock::DisplayMode displayMode = qApp->property(PROP_DISPLAY_MODE).value<Dock::DisplayMode>();
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    if (displayMode == Dock::Efficient) {
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, sw + "\n" + temp);
    } else {
        int w = qMin(width(), height());
        pixmap = pixmap.scaled(w*0.8, w*0.8, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        painter.drawPixmap(rect().center() - pixmap.rect().center(), pixmap);
    }
}