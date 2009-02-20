/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2007-12-03
 * Description : Greycstoration interface.
 *
 * Copyright (C) 2007-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#ifndef GREYCSTORATIONIFACE_H
#define GREYCSTORATIONIFACE_H

// Qt includes.

#include <QtGui/QImage>

// Local includes.

#include "dimg.h"
#include "dshareddata.h"
#include "dimgthreadedfilter.h"
#include "greycstorationsettings.h"
#include "digikam_export.h"

class QObject;

namespace Digikam
{

class GreycstorationIfacePriv;

class DIGIKAM_EXPORT GreycstorationIface : public DImgThreadedFilter
{

public:

    enum MODE
    {
        Restore = 0,
        InPainting,
        Resize,
        SimpleResize    // Mode to resize image without to use Greycstoration algorithm.
    };

public:

    /** Contructor without argument. Before to use it,
        you need to call in order: setSettings(), setMode(), optionally setInPaintingMask(), 
        setOriginalImage(), and necessary setup() at end.
     */
    GreycstorationIface(QObject *parent=0);

    /** Contructor with all arguments. Ready to use.
     */
    GreycstorationIface(DImg *orgImage,
                        const GreycstorationSettings& settings,
                        int mode=Restore,
                        int newWidth=0, int newHeight=0,
                        const QImage& inPaintingMask=QImage(),
                        QObject *parent=0);

    ~GreycstorationIface();

    /** Equivalent to the copy constructor */
    GreycstorationIface& operator=(const GreycstorationIface& iface);

    void setMode(int mode, int newWidth=0, int newHeight=0);
    void setSettings(const GreycstorationSettings& settings);
    void setInPaintingMask(const QImage& inPaintingMask);

    void setup();

    virtual void cancelFilter();

    static QString cimgVersionString();

private:

    virtual void initFilter();
    virtual void filterImage();

    void computeChildrenThreads();
    void restoration();
    void inpainting();
    void resize();
    void simpleResize();
    void iterationLoop(uint iter);

private:

    DSharedDataPointer<GreycstorationIfacePriv> m_priv;
};

}  // namespace Digikam

#endif /* GREYCSTORATIONIFACE_H */
