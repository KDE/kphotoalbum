#ifndef MD5CHECKPAGE_H
#define MD5CHECKPAGE_H

#include <QGridLayout>
#include "ImportSettings.h"
#include <QWidget>
#include "Utilities/Set.h"
class QButtonGroup;

namespace ImportExport
{

class ClashInfo
{
public:
    ClashInfo(const QStringList& categories);
    bool anyClashes();
    bool label;
    bool description;
    bool orientation;
    bool date;
    QMap<QString,bool> categories;
};

class MD5CheckPage :public QWidget
{
public:
    MD5CheckPage(const ImportSettings& settings);
    static bool pageNeeded( const ImportSettings& settings);
    QMap<QString, ImportSettings::ImportAction> settings();

private:
    void createRow( QGridLayout* layout, int& row, const QString& name, const QString& title, bool anyClashes, bool allowMerge );
    static int countOfMD5Matches(const ImportSettings& settings);
    static ClashInfo clashes(const ImportSettings& settings);
    static Utilities::StringSet mapCategoriesToDB( const CategoryMatchSetting& matcher, const Utilities::StringSet& items );

private:
    QMap<QString, QButtonGroup*> m_groups;
};

}

#endif /* MD5CHECKPAGE_H */

