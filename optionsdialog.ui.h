/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/

void OptionsDialog::show()
{
    Options* opt = Options::instance();

    thumbnailSize->setValue( opt->thumbSize() );
    cacheThumbNails->setChecked( opt->cacheThumbNails() );
    trustTimeStamps->setCurrentItem( opt->tTimeStamps() );
    imageDirectory->setText( opt->imageDirectory() );
    autosave->setValue( opt->autoSave() );
    QDialog::show();
}



void OptionsDialog::slotApply()
{
    Options* opt = Options::instance();
    opt->setThumbSize( thumbnailSize->value() );
    opt->setCacheThumbNails( cacheThumbNails->isChecked() );
    opt->setTTimeStamps( (Options::TimeStampTrust) trustTimeStamps->currentItem() );
    bool pathChanged = opt->imageDirectory() != imageDirectory->text();
    opt->setImageDirecotry( imageDirectory->text() );
    opt->setAutoSave( autosave->value() );
    opt->save();
    if ( pathChanged )
        emit imagePathChanged();
    else
        emit changed();
}


void OptionsDialog::slotBrowseForDirecory()
{
    QString dir = QFileDialog::getExistingDirectory( imageDirectory->text(), this );
    if ( ! dir.isNull() )
        imageDirectory->setText( dir );
}
