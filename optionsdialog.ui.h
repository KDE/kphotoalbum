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

    use4to3ratio->setChecked( opt->use4To3Ratio() );
    slotUpdate4To3View();

    thumbNailWidth->setValue( opt->thumbWidth() );
    thumbNailHeight->setValue( opt->thumbHeight() );
    cacheThumbNails->setChecked( opt->cacheThumbNails() );
    trustFileTimeStamps->setChecked( opt->trustFileTimeStamps() );

    QDialog::show();
}



void OptionsDialog::slotApply()
{
    Options* opt = Options::instance();
    opt->setThumbWidth( thumbNailWidth->value() );
    opt->setThumbHeight( thumbNailHeight->value() );
    opt->setCacheThumbNails( cacheThumbNails->isChecked() );
    opt->setTrustFileTimeStamps( trustFileTimeStamps->isChecked() );
    opt->save();
    emit changed();
}


void OptionsDialog::slotUpdate4To3View()
{
    thumbNailHeight->setEnabled( !use4to3ratio->isChecked() );
    heightLabel->setEnabled( !use4to3ratio->isChecked() );

}
