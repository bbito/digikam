/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2009-04-19
 * Description : Qt item view for images - the delegate
 *
 * Copyright (C) 2002-2005 by Renchi Raju <renchi dot raju at gmail dot com>
 * Copyright (C) 2002-2018 by Gilles Caulier <caulier dot gilles at gmail dot com>
 * Copyright (C) 2009-2011 by Andi Clemens <andi dot clemens at gmail dot com>
 * Copyright (C) 2006-2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef DIGIKAM_ITEMS_IMAGE_DELEGATE_PRIV_H
#define DIGIKAM_ITEMS_IMAGE_DELEGATE_PRIV_H

// Qt includes

#include <QRect>
#include <QCache>

// Local includes

#include "itemviewimagedelegatepriv.h"

namespace Digikam
{

class ImageCategoryDrawer;

class ImageDelegate::ImageDelegatePrivate : public ItemViewImageDelegatePrivate
{
public:

    ImageDelegatePrivate()
    {
        contentWidth        = 0;
        drawFocusFrame      = true;
        drawCoordinates     = false;
        drawImageFormat     = false;
        drawImageFormatTop  = false;
        drawMouseOverFrame  = true;
        ratingOverThumbnail = false;
        categoryDrawer      = 0;
        currentView         = 0;
        currentModel        = 0;

        actualPixmapRectCache.setMaxCost(250);
    }

    int                   contentWidth;

    QRect                 dateRect;
    QRect                 modDateRect;
    QRect                 pixmapRect;
    QRect                 specialInfoRect;
    QRect                 nameRect;
    QRect                 titleRect;
    QRect                 commentsRect;
    QRect                 resolutionRect;
    QRect                 arRect;
    QRect                 sizeRect;
    QRect                 tagRect;
    QRect                 imageInformationRect;
    QRect                 coordinatesRect;
    QRect                 pickLabelRect;
    QRect                 groupRect;

    bool                  drawFocusFrame;
    bool                  drawCoordinates;
    bool                  drawImageFormat;
    bool                  drawImageFormatTop;
    bool                  drawMouseOverFrame;
    bool                  ratingOverThumbnail;

    QCache<int, QRect>    actualPixmapRectCache;
    ImageCategoryDrawer*  categoryDrawer;

    ImageCategorizedView* currentView;
    QAbstractItemModel*   currentModel;

public:

    virtual void clearRects();
};

} // namespace Digikam

#endif // DIGIKAM_ITEMS_IMAGE_DELEGATE_PRIV_H

