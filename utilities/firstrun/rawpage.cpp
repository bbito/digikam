/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-28-04
 * Description : first run assistant dialog
 *
 * Copyright (C) 2009 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "rawpage.h"
#include "rawpage.moc"

// Qt includes

#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QVBoxLayout>

// KDE includes

#include <kdialog.h>
#include <kconfig.h>
#include <kvbox.h>
#include <kdebug.h>
#include <klocale.h>
#include <kstandarddirs.h>
#include <kapplication.h>

// Local includes

#include "version.h"

namespace Digikam
{

class RawPagePriv
{
public:

    RawPagePriv()
    {
        openDirectly = 0;
        useRawImport = 0;
        rawHandling  = 0;
    }

    QRadioButton *openDirectly;
    QRadioButton *useRawImport;

    QButtonGroup *rawHandling;
};

RawPage::RawPage(KAssistantDialog* dlg)
       : AssistantDlgPage(dlg, i18n("<b>Configure Raw Files Handling</b>")), 
         d(new RawPagePriv)
{
    KVBox *vbox   = new KVBox(this);
    QLabel *title = new QLabel(vbox);
    title->setWordWrap(true);
    title->setText(i18n("<qt>"
                        "<p>Configure Raw Files Handling.</p>"
                        "<p>Set here how you want to open Raw images in editor:</p>"
                        "</qt>"));

    QWidget *btns     = new QWidget(vbox);
    QVBoxLayout *vlay = new QVBoxLayout(btns);

    d->rawHandling    = new QButtonGroup(btns);
    d->openDirectly   = new QRadioButton(btns);
    d->openDirectly->setText(i18n("Open directy with automatic adjustements"));
    d->openDirectly->setChecked(true);
    d->rawHandling->addButton(d->openDirectly);

    d->useRawImport   = new QRadioButton(btns);
    d->useRawImport->setText(i18n("Use Raw import tool to adjust corrections manually (for advanced users)"));
    d->rawHandling->addButton(d->useRawImport);

    vlay->addWidget(d->openDirectly);
    vlay->addWidget(d->useRawImport);
    vlay->addStretch();
    vlay->setMargin(KDialog::spacingHint());
    vlay->setSpacing(KDialog::spacingHint());

    setPageWidget(vbox);
    setDigiKamLogo();
}

RawPage::~RawPage()
{
    delete d;
}

void RawPage::saveSettings()
{
    KSharedConfig::Ptr config = KGlobal::config();
    KConfigGroup group        = config->group(QString("ImageViewer Settings"));
    group.writeEntry("UseRawImportTool", d->useRawImport->isChecked());
    config->sync();
}

}   // namespace Digikam
