/* ============================================================
 * Authors: Gilles Caulier <caulier dot gilles at gmail dot com>
 *          Ralf Holzer <ralf at well.com>
 * Date   : 2003-08-03
 * Description : setup Metadata tab.
 * 
 * Copyright 2003-2004 by Ralf Holzer and Gilles Caulier
 * Copyright 2005-2006 by Gilles Caulier
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * ============================================================ */

// QT includes.

#include <qlayout.h>
#include <qvbuttongroup.h>
#include <qvgroupbox.h>
#include <qhgroupbox.h>
#include <qgroupbox.h>
#include <qcheckbox.h>
#include <qlabel.h>
#include <qwhatsthis.h>
#include <qtooltip.h>
#include <qhbox.h>

// KDE includes.

#include <klocale.h>
#include <kactivelabel.h>
#include <kdialog.h>
#include <kurllabel.h>
#include <kiconloader.h>
#include <kglobalsettings.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// // Local includes.

#include "albumsettings.h"
#include "setupmetadata.h"

namespace Digikam
{

class SetupMetadataPriv
{
public:

    SetupMetadataPriv()
    {
        ExifAutoRotateAsChanged   = false;
        saveCommentsBox           = 0;
        ExifRotateBox             = 0;
        ExifSetOrientationBox     = 0;
        saveRatingIptcBox         = 0;
        saveTagsIptcBox           = 0;
        saveDateTimeBox           = 0;
        savePhotographerIdIptcBox = 0;
        saveCreditsIptcBox        = 0;
    }

    bool       ExifAutoRotateAsChanged;
    bool       ExifAutoRotateOrg;
    
    QCheckBox *saveCommentsBox;
    QCheckBox *ExifRotateBox;
    QCheckBox *ExifSetOrientationBox;
    QCheckBox *saveRatingIptcBox;
    QCheckBox *saveTagsIptcBox;
    QCheckBox *saveDateTimeBox;
    QCheckBox *savePhotographerIdIptcBox;
    QCheckBox *saveCreditsIptcBox;
};

SetupMetadata::SetupMetadata(QWidget* parent )
             : QWidget(parent)
{
    d = new SetupMetadataPriv;
    QVBoxLayout *mainLayout = new QVBoxLayout(parent, 0, KDialog::spacingHint());

    // --------------------------------------------------------
  
    QGroupBox *ExifGroup = new QGroupBox(1, Qt::Horizontal, i18n("EXIF Actions"), parent);
  
    d->ExifRotateBox = new QCheckBox(ExifGroup);
    d->ExifRotateBox->setText(i18n("Show images/thumbs &rotated according to orientation tag"));
    
    d->ExifSetOrientationBox = new QCheckBox(ExifGroup);
    d->ExifSetOrientationBox->setText(i18n("Set orientation tag to normal after rotate/flip"));
    
    mainLayout->addWidget(ExifGroup);
  
    // --------------------------------------------------------
  
    QGroupBox *IptcGroup = new QGroupBox(1, Qt::Horizontal, i18n("IPTC Actions"), parent);

    d->saveTagsIptcBox = new QCheckBox(IptcGroup);
    d->saveTagsIptcBox->setText(i18n("&Save image tags as \"Keywords\" tag"));
    QWhatsThis::add( d->saveTagsIptcBox, i18n("<p>Turn this option on to store the image tags "
                                              "in the IPTC <i>Keywords</i> tag."));
  
    d->saveRatingIptcBox = new QCheckBox(IptcGroup);
    d->saveRatingIptcBox->setText(i18n("&Save image rating as \"Urgency\" tag"));
    QWhatsThis::add( d->saveRatingIptcBox, i18n("<p>Turn this option on to store the image rating "
                                                "in the IPTC <i>Urgency</i> tag."));

    d->savePhotographerIdIptcBox = new QCheckBox(IptcGroup);
    d->savePhotographerIdIptcBox->setText(i18n("&Save default photographer identity as tags"));
    QWhatsThis::add( d->savePhotographerIdIptcBox, i18n("<p>Turn this option on to store the default "
                                                        "photographer identity into the IPTC tags. You can set this "
                                                        "value in the Identity setup page."));

    d->saveCreditsIptcBox = new QCheckBox(IptcGroup);
    d->saveCreditsIptcBox->setText(i18n("&Save default credit and copyright identity as tags"));
    QWhatsThis::add( d->saveCreditsIptcBox, i18n("<p>Turn this option on to store the default "
                                                 "credit and copyright identity into the IPTC tags. "
                                                 "You can set this value in the Identity setup page."));
                                                           
    mainLayout->addWidget(IptcGroup);

    // --------------------------------------------------------
  
    QGroupBox *commonGroup = new QGroupBox(1, Qt::Horizontal, i18n("Common Metadata Actions"), parent);
  
    d->saveCommentsBox = new QCheckBox(commonGroup);
    d->saveCommentsBox->setText(i18n("&Save image comments as embedded text"));
    QWhatsThis::add( d->saveCommentsBox, i18n("<p>Turn this option on to store image comments "
                                              "into the JFIF section, EXIF tag, and IPTC tag."));

    d->saveDateTimeBox = new QCheckBox(commonGroup);
    d->saveDateTimeBox->setText(i18n("&Save image time stamp as tags"));
    QWhatsThis::add( d->saveDateTimeBox, i18n("<p>Turn this option on to store the image date and time "
                                              "into the EXIF and IPTC tags."));
    
    mainLayout->addWidget(commonGroup);
    mainLayout->addSpacing(KDialog::spacingHint());

    // --------------------------------------------------------
    
    QHBox *hbox = new QHBox(parent);

    KURLLabel *exiv2LogoLabel = new KURLLabel(hbox);
    exiv2LogoLabel->setText(QString::null);
    exiv2LogoLabel->setURL("http://www.exiv2.org");
    KGlobal::dirs()->addResourceType("logo-exiv2", KGlobal::dirs()->kde_default("data") + "digikam/data");
    QString directory = KGlobal::dirs()->findResourceDir("logo-exiv2", "logo-exiv2.png");
    exiv2LogoLabel->setPixmap( QPixmap( directory + "logo-exiv2.png" ) );
    QToolTip::add(exiv2LogoLabel, i18n("Visit Exiv2 project website"));

    KActiveLabel* explanation = new KActiveLabel(hbox);
    explanation->setText(i18n("<p><b>EXIF</b> is a standard used by most digital cameras today to store "
                              "technical information about the photograph as metadata in the image file. You can learn more "
                              "about EXIF at <a href='http://www.exif.org'>www.exif.org</a>.</p>"
                              "<p><b>IPTC</b> is another standard used in digital photography to store "
                              "embeded information in pictures. You can learn more "
                              "about IPTC at <a href='http://www.iptc.org/IIM'>www.iptc.org</a>.</p>"));
    
    mainLayout->addWidget(hbox);
    mainLayout->addStretch();
    mainLayout->addWidget(this);

    readSettings();
    adjustSize();
  
    // --------------------------------------------------------

    connect(exiv2LogoLabel, SIGNAL(leftClickedURL(const QString&)),
            this, SLOT(processExiv2URL(const QString&)));

    connect(d->ExifRotateBox, SIGNAL(toggled(bool)),
            this, SLOT(slotExifAutoRotateToggled(bool)));
}

SetupMetadata::~SetupMetadata()
{
    delete d;
}

void SetupMetadata::processExiv2URL(const QString& url)
{
    KApplication::kApplication()->invokeBrowser(url);
}

void SetupMetadata::applySettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    settings->setExifRotate(d->ExifRotateBox->isChecked());
    settings->setExifSetOrientation(d->ExifSetOrientationBox->isChecked());
    settings->setSaveComments(d->saveCommentsBox->isChecked());
    settings->setSaveDateTime(d->saveDateTimeBox->isChecked());
    settings->setSaveIptcRating(d->saveRatingIptcBox->isChecked());
    settings->setSaveIptcTags(d->saveTagsIptcBox->isChecked());
    settings->setSaveIptcPhotographerId(d->savePhotographerIdIptcBox->isChecked());
    settings->setSaveIptcCredits(d->saveCreditsIptcBox->isChecked());

    settings->saveSettings();
}

void SetupMetadata::readSettings()
{
    AlbumSettings* settings = AlbumSettings::instance();
    if (!settings) return;

    d->ExifAutoRotateOrg = settings->getExifRotate();
    d->ExifRotateBox->setChecked(d->ExifAutoRotateOrg);
    d->ExifSetOrientationBox->setChecked(settings->getExifSetOrientation());
    d->saveCommentsBox->setChecked(settings->getSaveComments());
    d->saveDateTimeBox->setChecked(settings->getSaveDateTime());
    d->saveRatingIptcBox->setChecked(settings->getSaveIptcRating());
    d->saveTagsIptcBox->setChecked(settings->getSaveIptcTags());
    d->savePhotographerIdIptcBox->setChecked(settings->getSaveIptcPhotographerId());
    d->saveCreditsIptcBox->setChecked(settings->getSaveIptcCredits());
}

bool SetupMetadata::exifAutoRotateAsChanged()
{
    return d->ExifAutoRotateAsChanged;
}

void SetupMetadata::slotExifAutoRotateToggled(bool b)
{
    if ( b != d->ExifAutoRotateOrg)
        d->ExifAutoRotateAsChanged = true;
    else
        d->ExifAutoRotateAsChanged = false;
}

}  // namespace Digikam

#include "setupmetadata.moc"
