// SPDX-FileCopyrightText: 2021 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
//
// SPDX-License-Identifier: LicenseRef-KDE-Accepted-GPL

#ifndef WILDCARDCATEGORYMATCHER_H
#define WILDCARDCATEGORYMATCHER_H

#include "SimpleCategoryMatcher.h"

#include <QRegularExpression>

namespace DB
{

/**
 * @brief The WildcardCategoryMatcher class matches any tag in any plain category.
 * It can be used to search over all categories when the user is not sure which category a tag belongs to.
 *
 * @note At initialisation, a list of categories and matching tags is compiled.
 * Therefore, the matcher can only match against tags that exist when it is initialized.
 */
class WildcardCategoryMatcher : public SimpleCategoryMatcher
{
public:
    /**
     * @brief regularExpression
     * @return the regular expression set by setRegularExpression
     */
    QRegularExpression regularExpression() const;
    /**
     * @brief Set a new match expression and compile a list of matching tags.
     * @param re a regular expression matching against the desired tags
     */
    void setRegularExpression(const QRegularExpression &re);
    bool eval(ImageInfoPtr, QMap<QString, StringSet> &alreadyMatched) override;
    /**
     * @brief evaluate the matcher for an image
     * @param info
     * @return \c true, if the ImageInfo is matched, \c false otherwise
     */
    bool eval(const ImageInfoPtr info) const;
    void debug(int level) const override;

private:
    QRegularExpression m_re;
    QHash<QString, StringSet> m_matchingTags; ///< The hash contains a list of matching tags for every category with matching tags.
};

}

#endif /* WILDCARDCATEGORYMATCHER_H */

// vi:expandtab:tabstop=4 shiftwidth=4:
