#ifndef HTMLGENERATOR_SETUP_H
#define HTMLGENERATOR_SETUP_H

#include <qstring.h>
#include <qmap.h>
#include "ImageSizeCheckBox.h"
#include <qvaluelist.h>

namespace HTMLGenerator
{
class Setup
{
public:
    void setTitle( const QString& title );
    QString title() const;

    void setBaseDir( const QString& baseDir );
    QString baseDir() const;

    void setBaseURL( const QString& baseURL );
    QString baseURL() const;

    void setDestURL( const QString& destURL );
    QString destURL() const;

    void setOutputDir( const QString& outputDir );
    QString outputDir() const;

    void setThumbSize( int thumbSize );
    int thumbSize() const;

    void setDescription( const QString& description );
    QString description() const;

    void setNumOfCols( int numOfCols );
    int numOfCols() const;

    void setGenerateKimFile( bool generateKimFile );
    bool generateKimFile() const;

    void setThemePath( const QString& theme );
    QString themePath() const;

    void setIncludeCategory( const QString& category, bool include );
    bool includeCategory( const QString& category ) const;

    void setResolutions( const QValueList<ImageSizeCheckBox*>& sizes );
    const QValueList<HTMLGenerator::ImageSizeCheckBox*>& activeResolutions() const;

    void setImageList( const QStringList& files );
    const QStringList& imageList() const;

    void setInlineMovies( bool inlineMovie );
    bool inlineMovies() const;


private:
    QString _title;
    QString _baseDir;
    QString _baseURL;
    QString _destURL;
    QString _outputDir;
    int _thumbSize;
    QString _description;
    int _numOfCols;
    bool _generateKimFile;
    QString _theme;
    QMap<QString,bool> _includeCategory;
    QValueList<ImageSizeCheckBox*> _resolutions;
    QStringList _images;
    bool _inlineMovies;
};

}

#endif /* HTMLGENERATOR_SETUP_H */

