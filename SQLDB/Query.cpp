#include "Query.h"
#include "DB/MemberMap.h"
#include "DB/ImageDB.h"
#include "DB/ImageSearchInfo.h"
#include <qsqlquery.h>
#include "Settings/SettingsData.h"
#include "QueryHelper.h"
#include <kdebug.h>

typedef QValueList<DB::OptionSimpleMatcher*> MathcerList;
typedef QValueList<MathcerList> MathcerListList;

namespace
{
QValueList<int> getMatchingFiles(MathcerList matches,
                                 int typemask=DB::anyMediaType);

QValueList<int> mergeUniqly(const QValueList<int>& l1,
                            const QValueList<int>& l2);
}

QValueList<int> SQLDB::filesMatchingQuery( const DB::ImageSearchInfo& info )
{
    QValueList< QValueList< DB::OptionSimpleMatcher*> > matches = info.query();
    // matches is in Disjunctive Normal Form ( OR(AND(a,b),AND(c,d)) )

    if ( matches.count() == 0 ) {
        return QueryHelper::instance()->allMediaItemIds();
    }

    QValueList<int> result;
    for( QValueList< QValueList<DB::OptionSimpleMatcher*> >::Iterator it = matches.begin(); it != matches.end(); ++it ) {
        result = mergeUniqly( result, runCategoryQuery( *it ) );
    }

    return result;
}

QValueList<int> SQLDB::searchMediaItems(const DB::ImageSearchInfo& search,
                                        int typemask)
{
    MathcerListList dnf = search.query();
    // dnf is in Disjunctive Normal Form ( OR(AND(a,b),AND(c,d)) )

    if (dnf.count() == 0) {
        return QueryHelper::instance()->allMediaItemIdsByType(typemask);
    }

    QValueList<int> r;
    for(MathcerListList::const_iterator i = dnf.begin(); i != dnf.end(); ++i) {
        r = mergeUniqly(r, getMatchingFiles(*i, typemask));
    }

    return r;
}

QValueList<int> SQLDB::runCategoryQuery( QValueList<DB::OptionSimpleMatcher*> matches )
{
    QValueList< DB::OptionSimpleMatcher*> possitiveList;
    QValueList< DB::OptionSimpleMatcher*> negativeList;
    split( matches, possitiveList, negativeList );

    return getMatchingFiles(matches);
}

namespace
{
using namespace SQLDB;

QValueList<int> getMatchingFiles(MathcerList matches, int typemask)
{
    MathcerList possitiveList;
    MathcerList negativeList;
    split(matches, possitiveList, negativeList);

    /*
    SELECT id FROM media
    WHERE
    id IN (SELECT mediaId FROM media_tag WHERE tagId IN (memberItem1TagIds))
    AND
    id IN (SELECT mediaId FROM media_tag WHERE tagId IN (memberItem2TagIds))
    AND ...
    */

    // Positive part of the query
    QStringList positiveQuery;
    QMap<QString, QValueList<int> > matchedTags;
    QStringList matchedFolders;
    QueryHelper::Bindings binds;
    for (MathcerList::const_iterator i = possitiveList.begin();
         i != possitiveList.end(); ++i) {
        DB::OptionValueMatcher* m = static_cast<DB::OptionValueMatcher*>(*i);
        if (m->_category == "Folder") {
            positiveQuery <<
                "id IN (SELECT media.id FROM media, dir "
                "WHERE media.dirId=dir.id AND dir.path=%s)";
            binds << m->_option;
            matchedFolders += m->_option;
        }
        else {
            positiveQuery <<
                "id IN (SELECT mediaId FROM media_tag WHERE tagId IN (%s))";
            QValueList<int> tagIds = QueryHelper::instance()->
                idListForTag(m->_category, m->_option);
            binds << toVariantList(tagIds);
            matchedTags[m->_category] += tagIds;
        }
    }

    // Negative query
    QStringList negativeQuery;
    QStringList excludeQuery;
    QueryHelper::Bindings excBinds;
    for (MathcerList::const_iterator i = negativeList.begin();
         i != negativeList.end(); ++i) {
        DB::OptionValueMatcher* m = dynamic_cast<DB::OptionValueMatcher*>(*i);
        if (m) {
            if (m->_category == "Folder") {
                negativeQuery <<
                    "id NOT IN (SELECT media.id FROM media, dir "
                    "WHERE media.dirId=dir.id AND dir.path=%s)";
                binds << m->_option;
            }
            else {
                negativeQuery <<
                    "id NOT IN (SELECT mediaId "
                    "FROM media_tag WHERE tagId IN (%s))";
                binds << toVariantList(QueryHelper::instance()->
                                       idListForTag(m->_category, m->_option));
            }
        }
        else {
            if ((*i)->_category == "Folder") {
                QStringList excludedFolders;
                if (matchedFolders.count() > 0) {
                    excludedFolders = matchedFolders;
                }
                else {
                    excludedFolders = QueryHelper::instance()->
                        executeQuery("SELECT path FROM dir").
                        asStringList();
                }
                if (excludedFolders.count() > 0) {
                    excludeQuery <<
                        "id IN (SELECT media.id FROM media, dir "
                        "WHERE media.dirId=dir.id AND dir.path IN (%s))";
                    excBinds << toVariantList(excludedFolders);
                }
            }
            else {
                QValueList<int> excludedTags;
                if (matchedTags[(*i)->_category].count() > 0) {
                    excludedTags = matchedTags[(*i)->_category];
                } else {
                    excludedTags =  QueryHelper::instance()->
                        tagIdsOfCategory((*i)->_category);
                }
                if (excludedTags.count() > 0) {
                    excludeQuery <<
                        "id IN (SELECT mediaId "
                        "FROM media_tag WHERE tagId IN (%s))";
                    excBinds << toVariantList(excludedTags);
                }
            }
        }
    }

    QString select = "SELECT id FROM media";
    QStringList condList = positiveQuery + negativeQuery;
    condList.prepend("media.type&%s!=0");
    binds.prepend(typemask);
    QString cond = condList.join(" AND ");

    QString query = select;
    if (cond.length() > 0)
        query += " WHERE " + cond;

    QValueList<int> positive = QueryHelper::instance()->
        executeQuery(query, binds).asIntegerList();

    if (excludeQuery.count() == 0)
        return positive;

    QValueList<int> negative = QueryHelper::instance()->
        executeQuery(select + " WHERE " + excludeQuery.join(" AND "),
                     excBinds).asIntegerList();

    return listSubstract(positive, negative);
}
}

/*
QString SQLDB::buildValue( const QString& category, const QStringList& vals, int idx, bool negate )
{
    QString expression;
    QString prefix;
    if ( idx != -1 )
        prefix = QString::fromLatin1("q%1.").arg( idx );

    if ( !vals.isEmpty() ) {
        for( QStringList::ConstIterator it = vals.begin(); it != vals.end(); ++it ) {
            if ( !expression.isEmpty() )
                expression += QString::fromLatin1( " or " );
            expression += QString::fromLatin1( "%1value = \"%2\"" ).arg( prefix ).arg( *it );
        }
        if ( negate )
            expression = QString::fromLatin1( "!(%1)" ).arg( expression );
        else
            expression = QString::fromLatin1( "(%1)" ).arg( expression );

        return QString::fromLatin1( "%1categoryId = \"%2\" and %3 " )
            .arg(prefix).arg( idForCategory(category) ).arg( expression );
    }
    else
        return QString::fromLatin1( "%1categoryId = \"%2\" " ).arg(prefix).arg( idForCategory(category) );
}
*/

QStringList SQLDB::values( DB::OptionValueMatcher* matcher )
{
    QStringList values;
    values.append( matcher->_option );

    if ( DB::ImageDB::instance()->memberMap().isGroup( matcher->_category, matcher->_option ) )
        values += DB::ImageDB::instance()->memberMap().members( matcher->_category, matcher->_option, true );
    return values;
}

void SQLDB::split( const QValueList<DB::OptionSimpleMatcher*>& input,
                   QValueList<DB::OptionSimpleMatcher*>& positiveList,
                   QValueList<DB::OptionSimpleMatcher*>& negativeList )
{
    for( QValueList<DB::OptionSimpleMatcher*>::ConstIterator it = input.constBegin(); it != input.constEnd(); ++it ) {
        DB::OptionValueMatcher* valueMatcher;
        if ( ( valueMatcher = dynamic_cast<DB::OptionValueMatcher*>( *it ) ) ) {
            if ( valueMatcher->_sign )
                positiveList.append( valueMatcher );
            else
                negativeList.append( valueMatcher );
        }
        else
            negativeList.append( *it );
    }
}

QString SQLDB::buildQueryPrefix( int count, int firstId )
{
    //  SELECT q0.fileId from imagecategoryinfo q0, imagecategoryinfo q1, ... WHERE q0.fileId = q1.fileId and q0.fileId = q1.fileId ....

    QStringList prefixes;
    QStringList matches;

    for ( int i = 0; i < count; ++i ) {
        prefixes << QString::fromLatin1( "imagecategoryinfo q%1" ).arg( i + firstId );
        if ( i != 0 )
            matches << QString::fromLatin1( "q0.fileId = q%1.fileId" ).arg( i + firstId );
    }

    return QString::fromLatin1( "SELECT q%1.fileId from %2 WHERE %3" ).arg(firstId).arg( prefixes.join( QString::fromLatin1( ", " ) ) )
        .arg( matches.join( QString::fromLatin1( " and " ) ) );
}

/*
QValueList<int> SQLDB::mergeUniqly( QValueList<int> l1, QValueList<int> l2)
{
    QValueList<int> result;
    l1 += l2;
    qHeapSort( l1 );

    QValueList<int>::Iterator it = l1.begin();
    int last = *it;
    result.append( last );
    ++it;

    for( ; it != l1.end(); ++it ) {
        if( *it != last ) {
            last = *it;
            result.append( last );
        }
    }
    return result;
}
*/

namespace
{
QValueList<int> mergeUniqly(const QValueList<int>& l1,
                            const QValueList<int>& l2)
{
    QValueList<int> r;
    const QValueList<int>* l[2] = {&l1, &l2};
    for (size_t n = 0; n < 2; ++n) {
        for (QValueList<int>::const_iterator i = l[n]->begin();
             i != l[n]->end(); ++i) {
            if (!r.contains(*i))
                r << *i;
        }
    }
    return r;
}
}

QValueList<int> SQLDB::listSubstract( QValueList<int> l1, QValueList<int> l2)
{
    qHeapSort(l1);
    qHeapSort(l2);
    QValueList<int>::Iterator it1 = l1.begin();
    QValueList<int>::Iterator it2 = l2.begin();

    QValueList<int> result;

    while ( it1 != l1.end() && it2 != l2.end()) {
        if ( *it1 == *it2 ) {
            ++it1; // There might be duplicates, so do not increment it2
        }

        else if ( *it1 < *it2 ) {
            result.append( *it1 );
            ++it1;
        }
        else {
            ++it2;
        }
    }

    while ( it1 != l1.end() ) {
        result.append( *it1 );
        ++it1;
    }

    return result;
}

