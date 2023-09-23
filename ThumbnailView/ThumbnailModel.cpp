// SPDX-FileCopyrightText: 2009-2022 Jesper K. Pedersen <jesper.pedersen@kdab.com>
// SPDX-FileCopyrightText: 2010 Jan Kundrát <jkt@flaska.net>
// SPDX-FileCopyrightText: 2010 Tuomas Suutari <tuomas@nepnep.net>
// SPDX-FileCopyrightText: 2012 Miika Turkia <miika.turkia@gmail.com>
// SPDX-FileCopyrightText: 2013-2023 Johannes Zarl-Zierl <johannes@zarl-zierl.at>
// SPDX-FileCopyrightText: 2015 Andreas Neustifter <andreas.neustifter@gmail.com>
// SPDX-FileCopyrightText: 2015-2022 Tobias Leupold <tl@stonemx.de>
//
// SPDX-License-Identifier: GPL-2.0-or-later

#include "ThumbnailModel.h"

#include "CellGeometry.h"
#include "FilterWidget.h"
#include "Logging.h"
#include "SelectionMaintainer.h"
#include "ThumbnailRequest.h"
#include "ThumbnailWidget.h"

#include <DB/ImageDB.h>
#include <ImageManager/AsyncLoader.h>
#include <Utilities/FileUtil.h>
#include <kpabase/FileName.h>
#include <kpabase/Logging.h>
#include <kpabase/SettingsData.h>
#include <kpathumbnails/ThumbnailCache.h>

#include <KLocalizedString>
#include <QElapsedTimer>
#include <QIcon>
#include <QLoggingCategory>

ThumbnailView::ThumbnailModel::ThumbnailModel(ThumbnailFactory *factory, const ImageManager::ThumbnailCache *thumbnailCache)
    : ThumbnailComponent(factory)
    , m_sortDirection(Settings::SettingsData::instance()->showNewestThumbnailFirst() ? NewestFirst : OldestFirst)
    , m_firstVisibleRow(-1)
    , m_lastVisibleRow(-1)
    , m_thumbnailCache(thumbnailCache)
{
    connect(DB::ImageDB::instance(), SIGNAL(imagesDeleted(DB::FileNameList)), this, SLOT(imagesDeletedFromDB(DB::FileNameList)));
    m_ImagePlaceholder = QIcon::fromTheme(QLatin1String("image-x-generic")).pixmap(cellGeometryInfo()->preferredIconSize());
    m_VideoPlaceholder = QIcon::fromTheme(QLatin1String("video-x-generic")).pixmap(cellGeometryInfo()->preferredIconSize());

    m_filter.setSearchMode(0);
    connect(this, &ThumbnailModel::filterChanged, this, &ThumbnailModel::updateDisplayModel);
    connect(m_thumbnailCache, &ImageManager::ThumbnailCache::thumbnailUpdated, this, qOverload<const DB::FileName &>(&ThumbnailModel::updateCell));
}

static bool stackOrderComparator(const DB::FileName &a, const DB::FileName &b)
{
    return DB::ImageDB::instance()->info(a)->stackOrder() < DB::ImageDB::instance()->info(b)->stackOrder();
}

void ThumbnailView::ThumbnailModel::updateDisplayModel()
{
    QElapsedTimer timer;
    timer.start();
    beginResetModel();
    ImageManager::AsyncLoader::instance()->stop(model(), ImageManager::StopOnlyNonPriorityLoads);

    // Note, this can be simplified, if we make the database backend already
    // return things in the right order. Then we only need one pass while now
    // we need to go through the list two times.

    /* Extract all stacks we have first. Different stackid's might be
     * intermingled in the result so we need to know this ahead before
     * creating the display list.
     */
    m_allStacks.clear();
    typedef QList<DB::FileName> StackList;
    typedef QMap<DB::StackID, StackList> StackMap;
    StackMap stackContents;
    for (const DB::FileName &fileName : qAsConst(m_imageList)) {
        const DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
        if (imageInfo && imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            m_allStacks << stackid;
            stackContents[stackid].append(fileName);
        }
    }

    /*
     * All stacks need to be ordered in their stack order. We don't rely that
     * the images actually came in the order necessary.
     */
    for (StackMap::iterator it = stackContents.begin(); it != stackContents.end(); ++it) {
        std::stable_sort(it->begin(), it->end(), stackOrderComparator);
    }

    /* Build the final list to be displayed. That is basically the sequence
     * we got from the original, but the stacks shown with all images together
     * in the right sequence or collapsed showing only the top image.
     */
    m_displayList = DB::FileNameList();
    QSet<DB::StackID> alreadyShownStacks;
    for (const DB::FileName &fileName : qAsConst(m_imageList)) {
        const DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
        if (!m_filter.match(imageInfo))
            continue;
        if (imageInfo && imageInfo->isStacked()) {
            DB::StackID stackid = imageInfo->stackId();
            if (alreadyShownStacks.contains(stackid))
                continue;
            StackMap::iterator found = stackContents.find(stackid);
            Q_ASSERT(found != stackContents.end());
            const StackList &orderedStack = *found;
            if (m_expandedStacks.contains(stackid)) {
                for (const DB::FileName &fileName : orderedStack) {
                    m_displayList.append(fileName);
                }
            } else {
                m_displayList.append(orderedStack.at(0));
            }
            alreadyShownStacks.insert(stackid);
        } else {
            m_displayList.append(fileName);
        }
    }

    if (m_sortDirection != OldestFirst)
        m_displayList = m_displayList.reversed();

    updateIndexCache();

    Q_EMIT collapseAllStacksEnabled(m_expandedStacks.size() > 0);
    Q_EMIT expandAllStacksEnabled(m_allStacks.size() != m_expandedStacks.size());
    endResetModel();
    qCInfo(TimingLog) << "ThumbnailModel::updateDisplayModel(): " << timer.restart() << "ms.";
}

void ThumbnailView::ThumbnailModel::toggleStackExpansion(const DB::FileName &fileName)
{
    const DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
    if (imageInfo) {
        DB::StackID stackid = imageInfo->stackId();
        beginResetModel();
        if (m_expandedStacks.contains(stackid))
            m_expandedStacks.remove(stackid);
        else
            m_expandedStacks.insert(stackid);
        endResetModel();
        updateDisplayModel();
    }
}

void ThumbnailView::ThumbnailModel::collapseAllStacks()
{
    m_expandedStacks.clear();
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::expandAllStacks()
{
    m_expandedStacks = m_allStacks;
    updateDisplayModel();
}

void ThumbnailView::ThumbnailModel::setImageList(const DB::FileNameList &items)
{
    m_imageList = items;
    updateDisplayModel();
    preloadThumbnails();
}

DB::FileNameList ThumbnailView::ThumbnailModel::imageList(Order order) const
{
    if (order == SortedOrder && m_sortDirection == NewestFirst)
        return m_displayList.reversed();
    else
        return m_displayList;
}

void ThumbnailView::ThumbnailModel::imagesDeletedFromDB(const DB::FileNameList &list)
{
    SelectionMaintainer dummy(widget(), model());

    for (const DB::FileName &fileName : list) {
        m_displayList.removeAll(fileName);
        m_imageList.removeAll(fileName);
    }
    updateDisplayModel();
}

int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName &fileName)
{
    Q_ASSERT(!fileName.isNull());
    if (!m_fileNameToIndex.contains(fileName))
        m_fileNameToIndex.insert(fileName, m_displayList.indexOf(fileName));

    return m_fileNameToIndex[fileName];
}

int ThumbnailView::ThumbnailModel::indexOf(const DB::FileName &fileName) const
{
    Q_ASSERT(!fileName.isNull());
    if (!m_fileNameToIndex.contains(fileName))
        return -1;

    return m_fileNameToIndex[fileName];
}

void ThumbnailView::ThumbnailModel::updateIndexCache()
{
    m_fileNameToIndex.clear();
    int index = 0;
    for (const DB::FileName &fileName : qAsConst(m_displayList)) {
        m_fileNameToIndex[fileName] = index;
        ++index;
    }
}

DB::FileName ThumbnailView::ThumbnailModel::rightDropItem() const
{
    return m_rightDrop;
}

void ThumbnailView::ThumbnailModel::setRightDropItem(const DB::FileName &item)
{
    m_rightDrop = item;
}

DB::FileName ThumbnailView::ThumbnailModel::leftDropItem() const
{
    return m_leftDrop;
}

void ThumbnailView::ThumbnailModel::setLeftDropItem(const DB::FileName &item)
{
    m_leftDrop = item;
}

void ThumbnailView::ThumbnailModel::setSortDirection(SortDirection direction)
{
    if (direction == m_sortDirection)
        return;

    Settings::SettingsData::instance()->setShowNewestFirst(direction == NewestFirst);
    m_displayList = m_displayList.reversed();
    updateIndexCache();

    m_sortDirection = direction;
}

bool ThumbnailView::ThumbnailModel::isItemInExpandedStack(const DB::StackID &id) const
{
    return m_expandedStacks.contains(id);
}

int ThumbnailView::ThumbnailModel::imageCount() const
{
    return m_displayList.size();
}

void ThumbnailView::ThumbnailModel::setOverrideImage(const DB::FileName &fileName, const QPixmap &pixmap)
{
    if (pixmap.isNull())
        m_overrideFileName = DB::FileName();
    else {
        m_overrideFileName = fileName;
        m_overrideImage = pixmap;
    }
    Q_EMIT dataChanged(fileNameToIndex(fileName), fileNameToIndex(fileName));
}

DB::FileName ThumbnailView::ThumbnailModel::imageAt(int index) const
{
    Q_ASSERT(index >= 0 && index < imageCount());
    return m_displayList.at(index);
}

int ThumbnailView::ThumbnailModel::rowCount(const QModelIndex &) const
{
    return imageCount();
}

QVariant ThumbnailView::ThumbnailModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_displayList.size())
        return QVariant();

    if (role == Qt::DecorationRole) {
        const DB::FileName fileName = m_displayList.at(index.row());
        return pixmap(fileName);
    }

    if (role == Qt::DisplayRole)
        return thumbnailText(index);

    return QVariant();
}

void ThumbnailView::ThumbnailModel::requestThumbnail(const DB::FileName &fileName, const ImageManager::Priority priority)
{
    const DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
    if (!imageInfo)
        return;
    // request the thumbnail in the size that is set in the settings, not in the current grid size:
    const QSize cellSize = cellGeometryInfo()->baseIconSize();
    const int angle = imageInfo->angle();
    const int row = indexOf(fileName);
    ThumbnailRequest *request
        = new ThumbnailRequest(row, fileName, cellSize, angle, this);
    request->setPriority(priority);
    ImageManager::AsyncLoader::instance()->load(request);
}

void ThumbnailView::ThumbnailModel::pixmapLoaded(ImageManager::ImageRequest *request, const QImage & /*image*/)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize fullSize = request->fullSize();

    // As a result of the image being loaded, we Q_EMIT the dataChanged signal, which in turn asks the delegate to paint the cell
    // The delegate now fetches the newly loaded image from the cache.

    DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
    // (hzeller): figure out, why the size is set here. We do an implicit
    // write here to the database.
    // (jzarl 2020-07-25): when loading a fullsize pixmap, we get the size "for free";
    // calculating it separately would cost us more than writing to the database from here.
    if (fullSize.isValid() && imageInfo) {
        imageInfo->setSize(fullSize);
    }

    Q_EMIT dataChanged(fileNameToIndex(fileName), fileNameToIndex(fileName));
}

QString ThumbnailView::ThumbnailModel::thumbnailText(const QModelIndex &index) const
{
    const DB::FileName fileName = imageAt(index.row());
    const auto info = DB::ImageDB::instance()->info(fileName);

    QString text;

    const QSize cellSize = cellGeometryInfo()->preferredIconSize();
    const int thumbnailHeight = cellSize.height() - 2 * Settings::SettingsData::instance()->thumbnailSpace();
    const int thumbnailWidth = cellSize.width(); // no subtracting here
    const int maxCharacters = thumbnailHeight / QFontMetrics(widget()->font()).maxWidth() * 2;

    if (Settings::SettingsData::instance()->displayLabels()) {
        QString line = info->label();
        if (widget()->fontMetrics().horizontalAdvance(line) > thumbnailWidth) {
            line = line.left(maxCharacters);
            line += QLatin1String(" ...");
        }
        text += line + QLatin1String("\n");
    }

    if (Settings::SettingsData::instance()->displayCategories()) {
        QStringList grps = info->availableCategories();
        grps.sort();
        for (QStringList::const_iterator it = grps.constBegin(); it != grps.constEnd(); ++it) {
            QString category = *it;
            if (category != i18n("Folder") && category != i18n("Media Type")) {
                Utilities::StringSet items = info->itemsOfCategory(category);

                if (DB::ImageDB::instance()->untaggedCategoryFeatureConfigured()
                    && !Settings::SettingsData::instance()->untaggedImagesTagVisible()) {

                    if (category == Settings::SettingsData::instance()->untaggedCategory()) {
                        if (items.contains(Settings::SettingsData::instance()->untaggedTag())) {
                            items.remove(Settings::SettingsData::instance()->untaggedTag());
                        }
                    }
                }

                if (!items.empty()) {
                    auto itemList = items.values();
                    itemList.sort();
                    QString line = itemList.join(QLatin1String(", "));

                    if (widget()->fontMetrics().horizontalAdvance(line) > thumbnailWidth) {
                        line = line.left(maxCharacters);
                        line += QLatin1String(" ...");
                    }
                    text += line + QLatin1String("\n");
                }
            }
        }
    }

    return text.trimmed();
}

void ThumbnailView::ThumbnailModel::updateCell(int row)
{
    updateCell(index(row, 0));
}

void ThumbnailView::ThumbnailModel::updateCell(const QModelIndex &index)
{
    Q_EMIT dataChanged(index, index);
}

void ThumbnailView::ThumbnailModel::updateCell(const DB::FileName &fileName)
{
    if (fileName.isNull())
        return;
    updateCell(indexOf(fileName));
}

QModelIndex ThumbnailView::ThumbnailModel::fileNameToIndex(const DB::FileName &fileName) const
{
    if (fileName.isNull())
        return QModelIndex();
    else
        return index(indexOf(fileName), 0);
}

QPixmap ThumbnailView::ThumbnailModel::pixmap(const DB::FileName &fileName) const
{
    if (m_overrideFileName == fileName)
        return m_overrideImage;

    const DB::ImageInfoPtr imageInfo = DB::ImageDB::instance()->info(fileName);
    if (imageInfo == DB::ImageInfoPtr(nullptr))
        return QPixmap();

    if (m_thumbnailCache->contains(fileName)) {
        // the cached thumbnail needs to be scaled to the actual thumbnail size:
        return m_thumbnailCache->lookup(fileName).scaled(cellGeometryInfo()->preferredIconSize(), Qt::KeepAspectRatio);
    }

    const_cast<ThumbnailView::ThumbnailModel *>(this)->requestThumbnail(fileName, ImageManager::ThumbnailVisible);
    if (imageInfo->isVideo())
        return m_VideoPlaceholder;
    else
        return m_ImagePlaceholder;
}

bool ThumbnailView::ThumbnailModel::isFiltered() const
{
    return !m_filter.isNull();
}

ThumbnailView::FilterWidget *ThumbnailView::ThumbnailModel::createFilterWidget(QWidget *parent)
{

    auto filterWidget = new FilterWidget(parent);
    connect(this, &ThumbnailModel::filterChanged, filterWidget, &FilterWidget::setFilter);
    connect(filterWidget, &FilterWidget::ratingChanged, this, &ThumbnailModel::filterByRating);
    connect(filterWidget, &FilterWidget::filterToggled, this, &ThumbnailModel::toggleFilter);
    return filterWidget;
}

bool ThumbnailView::ThumbnailModel::thumbnailStillNeeded(int row) const
{
    return (row >= m_firstVisibleRow && row <= m_lastVisibleRow);
}

void ThumbnailView::ThumbnailModel::updateVisibleRowInfo()
{
    m_firstVisibleRow = widget()->indexAt(QPoint(0, 0)).row();
    const int columns = widget()->width() / cellGeometryInfo()->cellSize().width();
    const int rows = widget()->height() / cellGeometryInfo()->cellSize().height();
    m_lastVisibleRow = qMin(m_firstVisibleRow + columns * (rows + 1), rowCount(QModelIndex()));

    // the cellGeometry has changed -> update placeholders
    m_ImagePlaceholder = QIcon::fromTheme(QLatin1String("image-x-generic")).pixmap(cellGeometryInfo()->preferredIconSize());
    m_VideoPlaceholder = QIcon::fromTheme(QLatin1String("video-x-generic")).pixmap(cellGeometryInfo()->preferredIconSize());
}

void ThumbnailView::ThumbnailModel::toggleFilter(bool enable)
{
    if (!enable)
        clearFilter();
    else if (m_filter.isNull()) {
        std::swap(m_filter, m_previousFilter);
        Q_EMIT filterChanged(m_filter);
    }
}

void ThumbnailView::ThumbnailModel::clearFilter()
{
    if (!m_filter.isNull()) {
        qCDebug(ThumbnailViewLog) << "Filter cleared.";
        m_previousFilter = m_filter;
        m_filter = DB::ImageSearchInfo();
        Q_EMIT filterChanged(m_filter);
    }
}

void ThumbnailView::ThumbnailModel::filterByRating(short rating)
{
    Q_ASSERT(-1 <= rating && rating <= 10);
    qCDebug(ThumbnailViewLog) << "Filter set: rating(" << rating << ")";
    m_filter.setRating(rating);
    Q_EMIT filterChanged(m_filter);
}

void ThumbnailView::ThumbnailModel::toggleRatingFilter(short rating)
{
    if (m_filter.rating() == rating) {
        filterByRating(rating);
    } else {
        filterByRating(-1);
        qCDebug(ThumbnailViewLog) << "Filter removed: rating";
        m_filter.setRating(-1);
        m_filter.checkIfNull();
        Q_EMIT filterChanged(m_filter);
    }
}

void ThumbnailView::ThumbnailModel::filterByCategory(const QString &category, const QString &tag)
{
    qCDebug(ThumbnailViewLog) << "Filter added: category(" << category << "," << tag << ")";

    m_filter.addAnd(category, tag);
    Q_EMIT filterChanged(m_filter);
}

void ThumbnailView::ThumbnailModel::toggleCategoryFilter(const QString &category, const QString &tag)
{
    auto tags = m_filter.categoryMatchText(category).split(QLatin1String("&"), Qt::SkipEmptyParts);
    for (const auto &existingTag : qAsConst(tags)) {
        if (tag == existingTag.trimmed()) {
            qCDebug(ThumbnailViewLog) << "Filter removed: category(" << category << "," << tag << ")";
            tags.removeAll(existingTag);
            m_filter.setCategoryMatchText(category, tags.join(QLatin1String(" & ")));
            m_filter.checkIfNull();
            Q_EMIT filterChanged(m_filter);
            return;
        }
    }
    filterByCategory(category, tag);
}

void ThumbnailView::ThumbnailModel::filterByFreeformText(const QString &text)
{
    qCDebug(ThumbnailViewLog) << "Filter added: freeform_match(" << text << ")";
    m_filter.setFreeformMatchText(text);
    Q_EMIT filterChanged(m_filter);
}

void ThumbnailView::ThumbnailModel::preloadThumbnails()
{
    // FIXME: it would make a lot of sense to merge preloadThumbnails() with pixmap()
    // and maybe also move the caching stuff into the ImageManager
    for (const DB::FileName &fileName : qAsConst(m_displayList)) {
        if (fileName.isNull())
            continue;

        if (m_thumbnailCache->contains(fileName))
            continue;
        const_cast<ThumbnailView::ThumbnailModel *>(this)->requestThumbnail(fileName, ImageManager::ThumbnailInvisible);
    }
}

// vi:expandtab:tabstop=4 shiftwidth=4:

#include "moc_ThumbnailModel.cpp"
