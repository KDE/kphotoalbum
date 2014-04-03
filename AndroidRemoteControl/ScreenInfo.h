#ifndef REMOTECONTROL_SCREENINFO_H
#define REMOTECONTROL_SCREENINFO_H

#include <QObject>
#include <QSize>
class QScreen;

namespace RemoteControl {

class ScreenInfo :public QObject
{
    Q_OBJECT
    Q_PROPERTY(double dotsPerMM MEMBER m_dotsPerMM CONSTANT)
    Q_PROPERTY(int overviewIconSize READ overviewIconSize WRITE setOverviewIconSize NOTIFY overviewIconSizeChanged)
    Q_PROPERTY(int overviewColumnCount MEMBER m_overviewColumnCount NOTIFY overviewColumnCountChanged)
    Q_PROPERTY(int overviewSpacing MEMBER m_overviewSpacing NOTIFY overviewSpacingChanged)
    Q_PROPERTY(int overviewScreenWidth READ overviewScreenWidth WRITE setOverviewScreenWidth NOTIFY overviewScreenWidthChanged)

public:
    static ScreenInfo& instance();
    void setScreen(QScreen*);
    QSize pixelForSizeInMM(int width, int height);
    void setCategoryCount(int count);
    QSize screenSize() const;

    int overviewIconSize() const;
    int overviewScreenWidth() const;

public slots:
    void setOverviewIconSize(int size);
    void setOverviewScreenWidth(int width);

private:
    void updateLayout();
signals:
    void overviewIconSizeChanged();    
    void overviewColumnCountChanged();
    void overviewSpacingChanged();
    void overviewScreenWidthChanged();

private:
    ScreenInfo() = default;
    QScreen* m_screen;
    double m_dotsPerMM;
    int m_categoryCount = 0;
    int m_overviewIconSize = 0;
    int m_overviewColumnCount = 0;
    int m_overviewSpacing = 0;
    int m_overviewScreenWidth = 0;
};

} // namespace RemoteControl

#endif // REMOTECONTROL_SCREENINFO_H
