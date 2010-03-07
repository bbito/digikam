/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2004-02-14
 * Description : a digiKam image plugin to apply special effects.
 *
 * Copyright (C) 2004-2010 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef IMAGEPLUGIN_FXFILTERS_H
#define IMAGEPLUGIN_FXFILTERS_H

// Qt includes

#include <QVariant>

// Local includes

#include "imageplugin.h"
#include "digikam_export.h"

class KAction;

using namespace Digikam;

class ImagePlugin_FxFilters : public ImagePlugin
{
    Q_OBJECT

public:

    ImagePlugin_FxFilters(QObject* parent, const QVariantList& args);
    ~ImagePlugin_FxFilters();

    void setEnabledActions(bool b);

private Q_SLOTS:

    void slotColorEffects();
    void slotCharcoal();
    void slotEmboss();
    void slotOilPaint();

private:

    KAction* m_oilpaintAction;
    KAction* m_embossAction;
    KAction* m_charcoalAction;
    KAction* m_colorEffectsAction;
};

#endif /* IMAGEPLUGIN_FXFILTERS_H */
