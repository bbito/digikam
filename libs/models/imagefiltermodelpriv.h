/* ============================================================
*
* This file is a part of digiKam project
* http://www.digikam.org
*
* Date        : 2009-03-11
* Description : Qt item model for database entries - private shared header
*
* Copyright (C) 2009 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
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

#ifndef IMAGEALBUMFILTERMODELPRIV_H
#define IMAGEALBUMFILTERMODELPRIV_H

// Qt includes.

#include <QMutex>
#include <QMutexLocker>
#include <QThread>
#include <QTimer>
#include <QWaitCondition>

// Local includes.

#include "digikam_export.h"
// Yes, we need the EXPORT macro in a private header because
// this private header is shared across binary objects.
// This does NOT make this classes here any more public!

namespace Digikam
{

class ImageFilterModelTodoPackage
{
public:

    ImageFilterModelTodoPackage()
        : version(0), isForReAdd(false) {}
    ImageFilterModelTodoPackage(const QVector<ImageInfo> &infos, int version, bool isForReAdd)
        : infos(infos), version(version), isForReAdd(isForReAdd) {}

    QVector<ImageInfo>         infos;
    unsigned int               version;
    bool                       isForReAdd;
    QHash<qlonglong, bool>     filterResults;
};

class ImageFilterModelPreparer;
class ImageFilterModelFilterer;

class DIGIKAM_EXPORT ImageFilterModelPrivate : public QObject
{
    Q_OBJECT

public:

    ImageFilterModelPrivate();
    ~ImageFilterModelPrivate();

    ImageFilterModel          *q;

    ImageModel                *imageModel;

    ImageFilterSettings        filter;

    volatile unsigned int      version;
    unsigned int               lastDiscardVersion;
    unsigned int               lastFilteredVersion;
    int                        sentOut;
    int                        sentOutForReAdd;

    QTimer                    *filterTimer;

    bool                       needPrepare;
    bool                       needPrepareComments;
    bool                       needPrepareTags;

    QMutex                     mutex;
    ImageFilterSettings        filterCopy;
    ImageFilterModelPreparer  *preparer;
    ImageFilterModelFilterer  *filterer;

    QHash<qlonglong, bool>     filterResults;

    void init(ImageFilterModel *q);
    void setupWorkers();
    void infosToProcess(const QList<ImageInfo> &infos, bool forReAdd);

public Q_SLOTS:

    void preprocessInfos(const QList<ImageInfo> &infos);
    void packageFinished(const ImageFilterModelTodoPackage &package);
    void packageDiscarded(const ImageFilterModelTodoPackage &package);

Q_SIGNALS:

    void packageToPrepare(const ImageFilterModelTodoPackage &package);
    void packageToFilter(const ImageFilterModelTodoPackage &package);
    void reAddImageInfos(const QList<ImageInfo> &infos);

};

}

#endif // IMAGEALBUMFILTERMODELPRIV_H

