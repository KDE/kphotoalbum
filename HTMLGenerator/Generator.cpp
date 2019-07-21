/* Copyright (C) 2003-2019 The KPhotoAlbum Development Team

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "Generator.h"

#include "ImageSizeCheckBox.h"
#include "Logging.h"
#include "Setup.h"

#include <DB/CategoryCollection.h>
#include <DB/ImageDB.h>
#include <DB/ImageInfo.h>
#include <Exif/Info.h>
#include <ImageManager/AsyncLoader.h>
#include <ImportExport/Export.h>
#include <MainWindow/Window.h>
#include <Utilities/FileUtil.h>
#include <Utilities/VideoUtil.h>

#include <KConfig>
#include <KConfigGroup>
#include <KIO/CopyJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KRun>
#include <QApplication>
#include <QDir>
#include <QDomDocument>
#include <QFile>
#include <QList>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <sys/types.h>
#include <sys/wait.h>

namespace
{
QString readFile(const QString &fileName)
{
    if (fileName.isEmpty()) {
        KMessageBox::error(nullptr, i18n("<p>No file name given!</p>"));
        return QString();
    }

    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        //KMessageBox::error( nullptr, i18n("Could not open file %1").arg( fileName ) );
        return QString();
    }

    QTextStream stream(&file);
    QString content = stream.readAll();
    file.close();

    return content;
}
} //namespace

HTMLGenerator::Generator::Generator(const Setup &setup, QWidget *parent)
    : QProgressDialog(parent)
    , m_tempDirHandle()
    , m_tempDir(m_tempDirHandle.path())
    , m_hasEnteredLoop(false)
{
    setLabelText(i18n("Generating images for HTML page "));
    m_setup = setup;
    m_eventLoop = new QEventLoop;
    m_avconv = QStandardPaths::findExecutable(QString::fromUtf8("avconv"));
    if (m_avconv.isNull())
        m_avconv = QStandardPaths::findExecutable(QString::fromUtf8("ffmpeg"));
    m_tempDirHandle.setAutoRemove(true);
}

HTMLGenerator::Generator::~Generator()
{
    delete m_eventLoop;
}
void HTMLGenerator::Generator::generate()
{
    qCDebug(HTMLGeneratorLog) << "Generating gallery" << m_setup.title() << "containing" << m_setup.imageList().size() << "entries in" << m_setup.baseDir();
    // Generate .kim file
    if (m_setup.generateKimFile()) {
        qCDebug(HTMLGeneratorLog) << "Generating .kim file...";
        bool ok;
        QString destURL = m_setup.destURL();

        ImportExport::Export exp(m_setup.imageList(), kimFileName(false),
                                 false, -1, ImportExport::ManualCopy,
                                 destURL + QDir::separator() + m_setup.outputDir(), true, &ok);
        if (!ok) {
            qCDebug(HTMLGeneratorLog) << ".kim file failed!";
            return;
        }
    }

    // prepare the progress dialog
    m_total = m_waitCounter = calculateSteps();
    setMaximum(m_total);
    setValue(0);
    connect(this, &QProgressDialog::canceled, this, &Generator::slotCancelGenerate);

    m_filenameMapper.reset();

    qCDebug(HTMLGeneratorLog) << "Generating content pages...";
    // Iterate over each of the image sizes needed.
    for (QList<ImageSizeCheckBox *>::ConstIterator sizeIt = m_setup.activeResolutions().begin();
         sizeIt != m_setup.activeResolutions().end(); ++sizeIt) {
        bool ok = generateIndexPage((*sizeIt)->width(), (*sizeIt)->height());
        if (!ok)
            return;
        const DB::FileNameList imageList = m_setup.imageList();
        for (int index = 0; index < imageList.size(); ++index) {
            DB::FileName current = imageList.at(index);
            DB::FileName prev;
            DB::FileName next;
            if (index != 0)
                prev = imageList.at(index - 1);
            if (index != imageList.size() - 1)
                next = imageList.at(index + 1);
            ok = generateContentPage((*sizeIt)->width(), (*sizeIt)->height(),
                                     prev, current, next);
            if (!ok)
                return;
        }
    }

    // Now generate the thumbnail images
    qCDebug(HTMLGeneratorLog) << "Generating thumbnail images...";
    for (const DB::FileName &fileName : m_setup.imageList()) {
        if (wasCanceled())
            return;

        createImage(fileName, m_setup.thumbSize());
    }

    if (wasCanceled())
        return;

    if (m_waitCounter > 0) {
        m_hasEnteredLoop = true;
        m_eventLoop->exec();
    }

    if (wasCanceled())
        return;

    qCDebug(HTMLGeneratorLog) << "Linking image file...";
    bool ok = linkIndexFile();
    if (!ok)
        return;

    qCDebug(HTMLGeneratorLog) << "Copying theme files...";
    // Copy over the mainpage.css, imagepage.css
    QString themeDir, themeAuthor, themeName;
    getThemeInfo(&themeDir, &themeName, &themeAuthor);
    QDir dir(themeDir);
    QStringList files = dir.entryList(QDir::Files);
    if (files.count() < 1)
        qCWarning(HTMLGeneratorLog) << QString::fromLatin1("theme '%1' doesn't have enough files to be a theme").arg(themeDir);

    for (QStringList::Iterator it = files.begin(); it != files.end(); ++it) {
        if (*it == QString::fromLatin1("kphotoalbum.theme") || *it == QString::fromLatin1("mainpage.html") || *it == QString::fromLatin1("imagepage.html"))
            continue;
        QString from = QString::fromLatin1("%1%2").arg(themeDir).arg(*it);
        QString to = m_tempDir.filePath(*it);
        ok = Utilities::copyOrOverwrite(from, to);
        if (!ok) {
            KMessageBox::error(this, i18n("Error copying %1 to %2", from, to));
            return;
        }
    }

    // Copy files over to destination.
    QString outputDir = m_setup.baseDir() + QString::fromLatin1("/") + m_setup.outputDir();
    qCDebug(HTMLGeneratorLog) << "Copying files from" << m_tempDir.path() << "to final location" << outputDir << "...";
    KIO::CopyJob *job = KIO::move(QUrl::fromLocalFile(m_tempDir.path()), QUrl::fromUserInput(outputDir));
    connect(job, &KIO::CopyJob::result, this, &Generator::showBrowser);

    m_eventLoop->exec();
    return;
}

bool HTMLGenerator::Generator::generateIndexPage(int width, int height)
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo(&themeDir, &themeName, &themeAuthor);
    QString content = readFile(QString::fromLatin1("%1mainpage.html").arg(themeDir));
    if (content.isEmpty())
        return false;

    // Adding the copyright comment after DOCTYPE not before (HTML standard requires the DOCTYPE to be first within the document)
    QRegExp rx(QString::fromLatin1("^(<!DOCTYPE[^>]*>)"));
    int position;

    rx.setCaseSensitivity(Qt::CaseInsensitive);
    position = rx.indexIn(content);
    if ((position += rx.matchedLength()) < 0)
        content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg(themeName).arg(themeAuthor) + content;
    else
        content.insert(position, QString::fromLatin1("\n<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg(themeName).arg(themeAuthor));

    content.replace(QString::fromLatin1("**DESCRIPTION**"), m_setup.description());
    content.replace(QString::fromLatin1("**TITLE**"), m_setup.title());

    QString copyright;
    if (!m_setup.copyright().isEmpty())
        copyright = QString::fromLatin1("&#169; %1").arg(m_setup.copyright());
    else
        copyright = QString::fromLatin1("&nbsp;");
    content.replace(QString::fromLatin1("**COPYRIGHT**"), copyright);

    QString kimLink = QString::fromLatin1("Share and Enjoy <a href=\"%1\">KPhotoAlbum export file</a>").arg(kimFileName(true));
    if (m_setup.generateKimFile())
        content.replace(QString::fromLatin1("**KIMFILE**"), kimLink);
    else
        content.remove(QString::fromLatin1("**KIMFILE**"));
    QDomDocument doc;

    QDomElement elm;
    QDomElement col;

    // -------------------------------------------------- Thumbnails
    // Initially all of the HTML generation was done using QDom, but it turned out in the end
    // to be much less code simply concatenating strings. This part, however, is easier using QDom
    // so we keep it using QDom.
    int count = 0;
    int cols = m_setup.numOfCols();
    int minWidth = 0;
    int minHeight = 0;
    int enableVideo = 0;
    QString first, last, images;

    images += QString::fromLatin1("var gallery=new Array()\nvar width=%1\nvar height=%2\nvar tsize=%3\nvar inlineVideo=%4\nvar generatedVideo=%5\n").arg(width).arg(height).arg(m_setup.thumbSize()).arg(m_setup.inlineMovies()).arg(m_setup.html5VideoGenerate());
    minImageSize(minWidth, minHeight);
    if (minWidth == 0 && minHeight == 0) { // full size only
        images += QString::fromLatin1("var minPage=\"index-fullsize.html\"\n");
    } else {
        images += QString::fromLatin1("var minPage=\"index-%1x%2.html\"\n").arg(minWidth).arg(minHeight);
    }

    QDomElement row;
    for (const DB::FileName &fileName : m_setup.imageList()) {
        const DB::ImageInfoPtr info = fileName.info();
        if (wasCanceled())
            return false;

        if (count % cols == 0) {
            row = doc.createElement(QString::fromLatin1("tr"));
            row.setAttribute(QString::fromLatin1("class"), QString::fromLatin1("thumbnail-row"));
            doc.appendChild(row);
            count = 0;
        }

        col = doc.createElement(QString::fromLatin1("td"));
        col.setAttribute(QString::fromLatin1("class"), QString::fromLatin1("thumbnail-col"));
        row.appendChild(col);

        if (first.isEmpty())
            first = namePage(width, height, fileName);
        else
            last = namePage(width, height, fileName);

        if (!Utilities::isVideo(fileName)) {
            QMimeDatabase db;
            images += QString::fromLatin1("gallery.push([\"%1\", \"%2\", \"%3\", \"%4\", \"")
                          .arg(nameImage(fileName, width))
                          .arg(nameImage(fileName, m_setup.thumbSize()))
                          .arg(nameImage(fileName, maxImageSize()))
                          .arg(db.mimeTypeForFile(nameImage(fileName, maxImageSize())).name());
        } else {
            QMimeDatabase db;
            images += QString::fromLatin1("gallery.push([\"%1\", \"%2\", \"%3\"")
                          .arg(nameImage(fileName, m_setup.thumbSize()))
                          .arg(nameImage(fileName, m_setup.thumbSize()))
                          .arg(nameImage(fileName, maxImageSize()));
            if (m_setup.html5VideoGenerate()) {
                images += QString::fromLatin1(", \"%1\", \"")
                              .arg(QString::fromLatin1("video/ogg"));
            } else {
                images += QString::fromLatin1(", \"%1\", \"")
                              .arg(db.mimeTypeForFile(fileName.relative(), QMimeDatabase::MatchExtension).name());
            }
            enableVideo = 1;
        }

        // -------------------------------------------------- Description
        if (!info->description().isEmpty() && m_setup.includeCategory(QString::fromLatin1("**DESCRIPTION**"))) {
            images += QString::fromLatin1("%1\", \"")
                          .arg(info->description()
                                   .replace(QString::fromLatin1("\n$"), QString::fromLatin1(""))
                                   .replace(QString::fromLatin1("\n"), QString::fromLatin1(" "))
                                   .replace(QString::fromLatin1("\""), QString::fromLatin1("\\\"")));
        } else {
            images += QString::fromLatin1("\", \"");
        }
        QString description = populateDescription(DB::ImageDB::instance()->categoryCollection()->categories(), info);

        if (!description.isEmpty()) {
            description = QString::fromLatin1("<ul>%1</ul>").arg(description);
        } else {
            description = QString::fromLatin1("");
        }

        description.replace(QString::fromLatin1("\n$"), QString::fromLatin1(""));
        description.replace(QString::fromLatin1("\n"), QString::fromLatin1(" "));
        description.replace(QString::fromLatin1("\""), QString::fromLatin1("\\\""));

        images += description;
        images += QString::fromLatin1("\"]);\n");

        QDomElement href = doc.createElement(QString::fromLatin1("a"));
        href.setAttribute(QString::fromLatin1("href"),
                          namePage(width, height, fileName));
        col.appendChild(href);

        QDomElement img = doc.createElement(QString::fromLatin1("img"));
        img.setAttribute(QString::fromLatin1("src"),
                         nameImage(fileName, m_setup.thumbSize()));
        img.setAttribute(QString::fromLatin1("alt"),
                         nameImage(fileName, m_setup.thumbSize()));
        href.appendChild(img);
        ++count;
    }

    // Adding TD elements to match the selected column amount for valid HTML
    if (count % cols != 0) {
        for (int i = count; i % cols != 0; ++i) {
            col = doc.createElement(QString::fromLatin1("td"));
            col.setAttribute(QString::fromLatin1("class"), QString::fromLatin1("thumbnail-col"));
            QDomText sp = doc.createTextNode(QString::fromLatin1(" "));
            col.appendChild(sp);
            row.appendChild(col);
        }
    }

    content.replace(QString::fromLatin1("**THUMBNAIL-TABLE**"), doc.toString());

    images += QString::fromLatin1("var enableVideo=%1\n").arg(enableVideo ? 1 : 0);
    content.replace(QString::fromLatin1("**JSIMAGES**"), images);
    if (!first.isEmpty())
        content.replace(QString::fromLatin1("**FIRST**"), first);
    if (!last.isEmpty())
        content.replace(QString::fromLatin1("**LAST**"), last);

    // -------------------------------------------------- Resolutions
    QString resolutions;
    QList<ImageSizeCheckBox *> actRes = m_setup.activeResolutions();
    std::sort(actRes.begin(), actRes.end());

    if (actRes.count() > 1) {
        resolutions += QString::fromLatin1("Resolutions: ");
        for (QList<ImageSizeCheckBox *>::ConstIterator sizeIt = actRes.constBegin();
             sizeIt != actRes.constEnd(); ++sizeIt) {

            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = QString::fromLatin1("index-%1.html").arg(ImageSizeCheckBox::text(w, h, true));
            QString text = (*sizeIt)->text(false);

            resolutions += QString::fromLatin1(" ");
            if (width == w && height == h) {
                resolutions += text;
            } else {
                resolutions += QString::fromLatin1("<a href=\"%1\">%2</a>").arg(page).arg(text);
            }
        }
    }

    content.replace(QString::fromLatin1("**RESOLUTIONS**"), resolutions);

    if (wasCanceled())
        return false;

    // -------------------------------------------------- write to file
    QString fileName = m_tempDir.filePath(
        QString::fromLatin1("index-%1.html")
            .arg(ImageSizeCheckBox::text(width, height, true)));
    bool ok = writeToFile(fileName, content);

    if (!ok)
        return false;

    return true;
}

bool HTMLGenerator::Generator::generateContentPage(int width, int height,
                                                   const DB::FileName &prev, const DB::FileName &current, const DB::FileName &next)
{
    QString themeDir, themeAuthor, themeName;
    getThemeInfo(&themeDir, &themeName, &themeAuthor);
    QString content = readFile(QString::fromLatin1("%1imagepage.html").arg(themeDir));
    if (content.isEmpty())
        return false;

    DB::ImageInfoPtr info = current.info();
    const DB::FileName currentFile = info->fileName();

    // Adding the copyright comment after DOCTYPE not before (HTML standard requires the DOCTYPE to be first within the document)
    QRegExp rx(QString::fromLatin1("^(<!DOCTYPE[^>]*>)"));
    int position;

    rx.setCaseSensitivity(Qt::CaseInsensitive);
    position = rx.indexIn(content);
    if ((position += rx.matchedLength()) < 0)
        content = QString::fromLatin1("<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg(themeName).arg(themeAuthor) + content;
    else
        content.insert(position, QString::fromLatin1("\n<!--\nMade with KPhotoAlbum. (http://www.kphotoalbum.org/)\nCopyright &copy; Jesper K. Pedersen\nTheme %1 by %2\n-->\n").arg(themeName).arg(themeAuthor));

    // TODO: Hardcoded non-standard category names is not good practice
    QString title = QString::fromLatin1("");
    QString name = QString::fromLatin1("Common Name");
    if (!info->itemsOfCategory(name).empty()) {
        title += QStringList(info->itemsOfCategory(name).toList()).join(QString::fromLatin1(" - "));
    } else {
        name = QString::fromLatin1("Latin Name");
        if (!info->itemsOfCategory(name).empty()) {
            title += QStringList(info->itemsOfCategory(name).toList()).join(QString::fromLatin1(" - "));
        } else {
            title = info->label();
        }
    }
    content.replace(QString::fromLatin1("**TITLE**"), title);

    // Image or video content
    if (Utilities::isVideo(currentFile)) {
        QString videoFile = createVideo(currentFile);
        QString videoBase = videoFile.replace(QRegExp(QString::fromLatin1("\\..*")), QString::fromLatin1(""));
        if (m_setup.inlineMovies())
            if (m_setup.html5Video())
                content.replace(QString::fromLatin1("**IMAGE_OR_VIDEO**"), QString::fromLatin1("<video controls><source src=\"%4\" type=\"video/mp4\" /><source src=\"%5\" type=\"video/ogg\" /><object data=\"%1\"><img src=\"%2\" alt=\"download\"/></object></video><a href=\"%3\"><img src=\"download.png\" /></a>").arg(QString::fromLatin1("%1.mp4").arg(videoBase)).arg(createImage(current, 256)).arg(QString::fromLatin1("%1.mp4").arg(videoBase)).arg(QString::fromLatin1("%1.mp4").arg(videoBase)).arg(QString::fromLatin1("%1.ogg").arg(videoBase)));
            else
                content.replace(QString::fromLatin1("**IMAGE_OR_VIDEO**"), QString::fromLatin1("<object data=\"%1\"><img src=\"%2\"/></object>"
                                                                                               "<a href=\"%3\"><img src=\"download.png\"/></a>")
                                                                               .arg(videoFile)
                                                                               .arg(createImage(current, 256))
                                                                               .arg(videoFile));
        else
            content.replace(QString::fromLatin1("**IMAGE_OR_VIDEO**"), QString::fromLatin1("<a href=\"**NEXTPAGE**\"><img src=\"%2\"/></a>"
                                                                                           "<a href=\"%1\"><img src=\"download.png\"/></a>")
                                                                           .arg(videoFile)
                                                                           .arg(createImage(current, 256)));
    } else
        content.replace(QString::fromLatin1("**IMAGE_OR_VIDEO**"),
                        QString::fromLatin1("<a href=\"**NEXTPAGE**\"><img src=\"%1\" alt=\"%1\"/></a>")
                            .arg(createImage(current, width)));

    // -------------------------------------------------- Links
    QString link;

    // prev link
    if (!prev.isNull())
        link = i18n("<a href=\"%1\">prev</a>", namePage(width, height, prev));
    else
        link = i18n("prev");
    content.replace(QString::fromLatin1("**PREV**"), link);

    // PENDING(blackie) These next 5 line also exists exactly like that in HTMLGenerator::Generator::generateIndexPage. Please refactor.
    // prevfile
    if (!prev.isNull())
        link = namePage(width, height, prev);
    else
        link = i18n("prev");
    content.replace(QString::fromLatin1("**PREVFILE**"), link);

    // index link
    link = i18n("<a href=\"index-%1.html\">index</a>", ImageSizeCheckBox::text(width, height, true));
    content.replace(QString::fromLatin1("**INDEX**"), link);

    // indexfile
    link = QString::fromLatin1("index-%1.html").arg(ImageSizeCheckBox::text(width, height, true));
    content.replace(QString::fromLatin1("**INDEXFILE**"), link);

    // Next Link
    if (!next.isNull())
        link = i18n("<a href=\"%1\">next</a>", namePage(width, height, next));
    else
        link = i18n("next");
    content.replace(QString::fromLatin1("**NEXT**"), link);

    // Nextfile
    if (!next.isNull())
        link = namePage(width, height, next);
    else
        link = i18n("next");
    content.replace(QString::fromLatin1("**NEXTFILE**"), link);

    if (!next.isNull())
        link = namePage(width, height, next);
    else
        link = QString::fromLatin1("index-%1.html").arg(ImageSizeCheckBox::text(width, height, true));

    content.replace(QString::fromLatin1("**NEXTPAGE**"), link);

    // -------------------------------------------------- Resolutions
    QString resolutions;
    const QList<ImageSizeCheckBox *> &actRes = m_setup.activeResolutions();
    if (actRes.count() > 1) {
        for (QList<ImageSizeCheckBox *>::ConstIterator sizeIt = actRes.begin();
             sizeIt != actRes.end(); ++sizeIt) {
            int w = (*sizeIt)->width();
            int h = (*sizeIt)->height();
            QString page = namePage(w, h, currentFile);
            QString text = (*sizeIt)->text(false);
            resolutions += QString::fromLatin1(" ");

            if (width == w && height == h)
                resolutions += text;
            else
                resolutions += QString::fromLatin1("<a href=\"%1\">%2</a>").arg(page).arg(text);
        }
    }
    content.replace(QString::fromLatin1("**RESOLUTIONS**"), resolutions);

    // -------------------------------------------------- Copyright
    QString copyright;

    if (!m_setup.copyright().isEmpty())
        copyright = QString::fromLatin1("&#169; %1").arg(m_setup.copyright());
    else
        copyright = QString::fromLatin1("&nbsp;");
    content.replace(QString::fromLatin1("**COPYRIGHT**"), QString::fromLatin1("%1").arg(copyright));

    // -------------------------------------------------- Description
    QString description = populateDescription(DB::ImageDB::instance()->categoryCollection()->categories(), info);

    if (!description.isEmpty())
        content.replace(QString::fromLatin1("**DESCRIPTION**"), QString::fromLatin1("<ul>\n%1\n</ul>").arg(description));
    else
        content.replace(QString::fromLatin1("**DESCRIPTION**"), QString::fromLatin1(""));

    // -------------------------------------------------- write to file
    QString fileName = m_tempDir.filePath(namePage(width, height, currentFile));
    bool ok = writeToFile(fileName, content);
    if (!ok)
        return false;

    return true;
}

QString HTMLGenerator::Generator::namePage(int width, int height, const DB::FileName &fileName)
{
    QString name = m_filenameMapper.uniqNameFor(fileName);
    QString base = QFileInfo(name).completeBaseName();
    return QString::fromLatin1("%1-%2.html").arg(base).arg(ImageSizeCheckBox::text(width, height, true));
}

QString HTMLGenerator::Generator::nameImage(const DB::FileName &fileName, int size)
{
    QString name = m_filenameMapper.uniqNameFor(fileName);
    QString base = QFileInfo(name).completeBaseName();
    if (size == maxImageSize() && !Utilities::isVideo(fileName)) {
        if (name.endsWith(QString::fromLatin1(".jpg"), Qt::CaseSensitive) || name.endsWith(QString::fromLatin1(".jpeg"), Qt::CaseSensitive))
            return name;
        else
            return base + QString::fromLatin1(".jpg");
    } else if (size == maxImageSize() && Utilities::isVideo(fileName)) {
        return name;
    } else
        return QString::fromLatin1("%1-%2.jpg").arg(base).arg(size);
}

QString HTMLGenerator::Generator::createImage(const DB::FileName &fileName, int size)
{
    DB::ImageInfoPtr info = fileName.info();
    if (m_generatedFiles.contains(qMakePair(fileName, size))) {
        m_waitCounter--;
    } else {
        ImageManager::ImageRequest *request = new ImageManager::ImageRequest(fileName, QSize(size, size),
                                                                             info->angle(), this);
        request->setPriority(ImageManager::BatchTask);
        ImageManager::AsyncLoader::instance()->load(request);
        m_generatedFiles.insert(qMakePair(fileName, size));
    }

    return nameImage(fileName, size);
}

QString HTMLGenerator::Generator::createVideo(const DB::FileName &fileName)
{
    setValue(m_total - m_waitCounter);
    qApp->processEvents();

    QString baseName = nameImage(fileName, maxImageSize());
    QString destName = m_tempDir.filePath(baseName);
    if (!m_copiedVideos.contains(fileName)) {
        if (m_setup.html5VideoGenerate()) {
            // TODO: shouldn't we use avconv library directly instead of KRun
            // TODO: should check that the avconv (ffmpeg takes the same parameters on older systems) and ffmpeg2theora exist
            // TODO: Figure out avconv parameters to get rid of ffmpeg2theora
            KRun::runCommand(QString::fromLatin1("%1 -y -i %2  -vcodec libx264 -b 250k -bt 50k -acodec libfaac -ab 56k -ac 2 -s %3 %4")
                                 .arg(m_avconv)
                                 .arg(fileName.absolute())
                                 .arg(QString::fromLatin1("320x240"))
                                 .arg(destName.replace(QRegExp(QString::fromLatin1("\\..*")), QString::fromLatin1(".mp4"))),
                             MainWindow::Window::theMainWindow());
            KRun::runCommand(QString::fromLatin1("ffmpeg2theora -v 7 -o %1 -x %2 %3")
                                 .arg(destName.replace(QRegExp(QString::fromLatin1("\\..*")), QString::fromLatin1(".ogg")))
                                 .arg(QString::fromLatin1("320"))
                                 .arg(fileName.absolute()),
                             MainWindow::Window::theMainWindow());
        } else
            Utilities::copyOrOverwrite(fileName.absolute(), destName);
        m_copiedVideos.insert(fileName);
    }
    return baseName;
}

QString HTMLGenerator::Generator::kimFileName(bool relative)
{
    if (relative)
        return QString::fromLatin1("%2.kim").arg(m_setup.outputDir());
    else
        return m_tempDir.filePath(QString::fromLatin1("%2.kim").arg(m_setup.outputDir()));
}

bool HTMLGenerator::Generator::writeToFile(const QString &fileName, const QString &str)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        KMessageBox::error(this, i18n("Could not create file '%1'.", fileName),
                           i18n("Could Not Create File"));
        return false;
    }

    QByteArray data = translateToHTML(str).toUtf8();
    file.write(data);
    file.close();
    return true;
}

QString HTMLGenerator::Generator::translateToHTML(const QString &str)
{
    QString res;
    for (int i = 0; i < str.length(); ++i) {
        if (str[i].unicode() < 128)
            res.append(str[i]);
        else {
            res.append(QString().sprintf("&#%u;", (unsigned int)str[i].unicode()));
        }
    }
    return res;
}

bool HTMLGenerator::Generator::linkIndexFile()
{
    ImageSizeCheckBox *resolution = m_setup.activeResolutions()[0];
    QString fromFile = QString::fromLatin1("index-%1.html")
                           .arg(resolution->text(true));
    fromFile = m_tempDir.filePath(fromFile);
    QString destFile = m_tempDir.filePath(QString::fromLatin1("index.html"));
    bool ok = Utilities::copyOrOverwrite(fromFile, destFile);
    if (!ok) {
        KMessageBox::error(this, i18n("<p>Unable to copy %1 to %2</p>", fromFile, destFile));

        return false;
    }
    return ok;
}

void HTMLGenerator::Generator::slotCancelGenerate()
{
    ImageManager::AsyncLoader::instance()->stop(this);
    m_waitCounter = 0;
    if (m_hasEnteredLoop)
        m_eventLoop->exit();
}

void HTMLGenerator::Generator::pixmapLoaded(ImageManager::ImageRequest *request, const QImage &image)
{
    const DB::FileName fileName = request->databaseFileName();
    const QSize imgSize = request->size();
    const bool loadedOK = request->loadedOK();

    setValue(m_total - m_waitCounter);

    m_waitCounter--;

    int size = imgSize.width();
    QString file = m_tempDir.filePath(nameImage(fileName, size));

    bool success = loadedOK && image.save(file, "JPEG");
    if (!success) {
        // We better stop the imageloading. In case this is a full disk, we will just get all images loaded, while this
        // error box is showing, resulting in a bunch of error messages, and memory running out due to all the hanging
        // pixmapLoaded methods.
        slotCancelGenerate();
        KMessageBox::error(this, i18n("Unable to write image '%1'.", file));
    }

    if (!Utilities::isVideo(fileName)) {
        try {
            Exif::Info::instance()->writeInfoToFile(fileName, file);
        } catch (...) {
        }
    }

    if (m_waitCounter == 0 && m_hasEnteredLoop) {
        m_eventLoop->exit();
    }
}

int HTMLGenerator::Generator::calculateSteps()
{
    int count = m_setup.activeResolutions().count();
    return m_setup.imageList().size() * (1 + count); // 1 thumbnail + 1 real image
}

void HTMLGenerator::Generator::getThemeInfo(QString *baseDir, QString *name, QString *author)
{
    *baseDir = m_setup.themePath();
    KConfig themeconfig(QString::fromLatin1("%1/kphotoalbum.theme").arg(*baseDir), KConfig::SimpleConfig);
    KConfigGroup config = themeconfig.group("theme");

    *name = config.readEntry("Name");
    *author = config.readEntry("Author");
}

int HTMLGenerator::Generator::maxImageSize()
{
    int res = 0;
    for (QList<ImageSizeCheckBox *>::ConstIterator sizeIt = m_setup.activeResolutions().begin();
         sizeIt != m_setup.activeResolutions().end(); ++sizeIt) {
        res = qMax(res, (*sizeIt)->width());
    }
    return res;
}

void HTMLGenerator::Generator::minImageSize(int &width, int &height)
{
    width = height = 0;
    for (QList<ImageSizeCheckBox *>::ConstIterator sizeIt = m_setup.activeResolutions().begin();
         sizeIt != m_setup.activeResolutions().end(); ++sizeIt) {
        if ((width == 0) && ((*sizeIt)->width() > 0)) {
            width = (*sizeIt)->width();
            height = (*sizeIt)->height();
        } else if ((*sizeIt)->width() > 0) {
            width = qMin(width, (*sizeIt)->width());
            height = qMin(height, (*sizeIt)->height());
        }
    }
}

void HTMLGenerator::Generator::showBrowser()
{
    if (m_setup.generateKimFile())
        ImportExport::Export::showUsageDialog();

    if (!m_setup.baseURL().isEmpty())
        new KRun(QUrl::fromUserInput(QString::fromLatin1("%1/%2/index.html").arg(m_setup.baseURL()).arg(m_setup.outputDir())),
                 MainWindow::Window::theMainWindow());

    m_eventLoop->exit();
}

QString HTMLGenerator::Generator::populateDescription(QList<DB::CategoryPtr> categories, const DB::ImageInfoPtr info)
{
    QString description;

    if (m_setup.includeCategory(QString::fromLatin1("**DATE**")))
        description += QString::fromLatin1("<li> <b>%1</b> %2</li>").arg(i18n("Date")).arg(info->date().toString());

    for (QList<DB::CategoryPtr>::Iterator it = categories.begin(); it != categories.end(); ++it) {
        if ((*it)->isSpecialCategory())
            continue;

        QString name = (*it)->name();
        if (!info->itemsOfCategory(name).empty() && m_setup.includeCategory(name)) {
            QString val = QStringList(info->itemsOfCategory(name).toList()).join(QString::fromLatin1(", "));
            description += QString::fromLatin1("  <li> <b>%1:</b> %2</li>").arg(name).arg(val);
        }
    }

    if (!info->description().isEmpty() && m_setup.includeCategory(QString::fromLatin1("**DESCRIPTION**"))) {
        description += QString::fromLatin1("  <li> <b>Description:</b> %1</li>").arg(info->description());
    }

    return description;
}

// vi:expandtab:tabstop=4 shiftwidth=4:
