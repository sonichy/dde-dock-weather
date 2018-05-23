#ifndef WEATHERPlugin_H
#define WEATHERPlugin_H

#include "pluginsiteminterface.h"
#include "weatherwidget.h"
#include "forcastwidget.h"
#include <QTimer>
#include <QLabel>
#include <QGridLayout>
#include <DBlurEffectWidget>

DWIDGET_USE_NAMESPACE

class WeatherPlugin : public QObject, PluginsItemInterface
{
    Q_OBJECT
    Q_INTERFACES(PluginsItemInterface)
    Q_PLUGIN_METADATA(IID "com.deepin.dock.PluginsItemInterface" FILE "weather.json")

public:
    explicit WeatherPlugin(QObject *parent = 0);

    const QString pluginName() const override;
    const QString pluginDisplayName() const override;
    void init(PluginProxyInterface *proxyInter) override;

    void pluginStateSwitched() override;
    bool pluginIsAllowDisable() override { return true; }
    bool pluginIsDisable() override;

    int itemSortKey(const QString &itemKey) override;

    QWidget *itemWidget(const QString &itemKey) override;
    QWidget *itemTipsWidget(const QString &itemKey) override;    
    QWidget *itemPopupApplet(const QString &itemKey);

    const QString itemContextMenu(const QString &itemKey) override;

    void invokedMenuItem(const QString &itemKey, const QString &menuId, const bool checked) override;

private slots:
    void weatherNow(QString weather, QString temp, QString stip, QPixmap pixmap);

private:    
    QPointer<WeatherWidget> m_centralWidget;
    QPointer<QLabel> m_tipsLabel;
    QTimer *m_refershTimer;
    void MBAbout();
    ForcastWidget *forcastApplet;
};

#endif // WEATHERPlugin_H
