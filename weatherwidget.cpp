#include "dde-dock/constants.h"
#include "weatherwidget.h"
#include <QApplication>
#include <QPainter>
#include <QDebug>
#include <QSvgRenderer>
#include <QMouseEvent>

#define PLUGIN_STATE_KEY    "enable"

WeatherWidget::WeatherWidget(QWidget *parent)
    : QWidget(parent),
    m_settings("deepin", "dde-dock-HTYWeather")
{
    sw = "天气";
    temp = "温度";
    pixmap = QPixmap(":icon/NA.png");
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
        if(FM.boundingRect(sw + "  ").width() >= FM.boundingRect(temp + "  ").width()){
            size = FM.boundingRect(sw).size() + QSize(0,FM.boundingRect(sw).height());
        }else{
            size = FM.boundingRect(temp).size() + QSize(0,FM.boundingRect(temp).height());
        }
    }else{
        FM.boundingRect("天气").size();
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
    painter.setPen(Qt::white);
    if (displayMode == Dock::Efficient) {
        painter.drawText(rect(), Qt::AlignCenter, " " +sw + "\n " + temp);
    } else {
        painter.drawPixmap(rect(), pixmap.scaled(rect().size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

void WeatherWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() != Qt::RightButton)
        return QWidget::mousePressEvent(e);

    const QPoint p(e->pos() - rect().center());
    if (p.manhattanLength() < std::min(width(), height()) * 0.8 * 0.5)
    {
        emit requestContextMenu();
        return;
    }

    QWidget::mousePressEvent(e);
}
