#include "QueryHelper.h"
#include "DB/ImageSearchInfo.h"
#include "DB/CategoryMatcher.h"
#include "Utilities/List.h"

using namespace SQLDB;
using Utilities::mergeListsUniqly;
using Utilities::listSubtract;

namespace
{
    void split(const MatcherList& input,
               MatcherList& positiveList, MatcherList& negativeList)
    {
        for (MatcherList::const_iterator it = input.constBegin();
             it != input.constEnd(); ++it) {
            DB::OptionValueMatcher* valueMatcher;
            valueMatcher = dynamic_cast<DB::OptionValueMatcher*>(*it);
            if (valueMatcher) {
                if (valueMatcher->_sign)
                    positiveList.append(valueMatcher);
                else
                    negativeList.append(valueMatcher);
            }
            else
                negativeList.append(*it);
        }
    }
}

QValueList<int>
QueryHelper::searchMediaItems(const DB::ImageSearchInfo& search, int typemask)
{
    MatcherListList dnf = search.query();
    // dnf is in Disjunctive Normal Form ( OR(AND(a,b),AND(c,d)) )

    if (dnf.count() == 0) {
        return allMediaItemIdsByType(typemask);
    }

    QValueList<int> r;
    for(MatcherListList::const_iterator i = dnf.begin(); i != dnf.end(); ++i) {
        r = mergeListsUniqly(r, getMatchingFiles(*i, typemask));
    }

    return r;
}

QValueList<int>
QueryHelper::getMatchingFiles(MatcherList matches, int typemask)
{
    MatcherList positiveList;
    MatcherList negativeList;
    split(matches, positiveList, negativeList);

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
    Bindings binds;
    for (MatcherList::const_iterator i = positiveList.begin();
         i != positiveList.end(); ++i) {
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
            QValueList<int> tagIds = idListForTag(m->_category, m->_option);
            binds << toVariantList(tagIds);
            matchedTags[m->_category] += tagIds;
        }
    }

    // Negative query
    QStringList negativeQuery;
    QStringList excludeQuery;
    Bindings excBinds;
    for (MatcherList::const_iterator i = negativeList.begin();
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
                binds << toVariantList(idListForTag(m->_category, m->_option));
            }
        }
        else {
            if ((*i)->_category == "Folder") {
                QStringList excludedFolders;
                if (matchedFolders.count() > 0) {
                    excludedFolders = matchedFolders;
                }
                else {
                    excludedFolders = executeQuery("SELECT path FROM dir").
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
                    excludedTags = tagIdsOfCategory((*i)->_category);
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
    if (typemask != DB::anyMediaType) {
        condList.prepend("media.type&%s!=0");
        binds.prepend(typemask);
    }
    QString cond = condList.join(" AND ");

    QString query = select;
    if (cond.length() > 0)
        query += " WHERE " + cond;

    query += " ORDER BY place";

    QValueList<int> positive = executeQuery(query, binds).asIntegerList();

    if (excludeQuery.count() == 0)
        return positive;

    QValueList<int> negative =
        executeQuery(select + " WHERE " + excludeQuery.join(" AND "),
                     excBinds).asIntegerList();

    return listSubtract(positive, negative);
}

