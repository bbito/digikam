/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2011-11-07
 * Description : Directory watch interface
 *
 * Copyright (C) 2011 by Marcel Wiesweg <marcel dot wiesweg at gmx dot de>
 * Copyright (C) 2015 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "albumwatch.h"

// Qt includes

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QFileSystemWatcher>

// Local includes

#include "digikam_debug.h"
#include "album.h"
#include "albummanager.h"
#include "collectionlocation.h"
#include "collectionmanager.h"
#include "databaseparameters.h"
#include "scancontroller.h"

#ifdef USE_KNOTIFY
#include "kinotify.h"
#endif

namespace Digikam
{

enum Mode
{
    InotifyMode,
    QFSWatcherMode
};

class AlbumWatch::Private
{
public:

    explicit Private(AlbumWatch* const q)
        : mode(InotifyMode),
#ifdef USE_KNOTIFY
          inotify(0),
#endif
          dirWatch(0),
          q(q)
    {
    }

    void             determineMode();
    bool             isInotifyMode()                  const;
    bool             inBlackList(const QString& path) const;
    bool             inDirWatchParametersBlackList(const QFileInfo& info, const QString& path);
    QList<QDateTime> buildDirectoryModList(const QFileInfo& dbFile) const;

public:

    Mode                mode;

#ifdef USE_KNOTIFY
    KInotify*           inotify;
#endif

    QFileSystemWatcher* dirWatch;
    QStringList         dirWatchAddedDirs;

    DatabaseParameters  params;
    QStringList         fileNameBlackList;
    QList<QDateTime>    dbPathModificationDateList;

    AlbumWatch* const   q;
};

bool AlbumWatch::Private::isInotifyMode() const
{
    return (mode == InotifyMode);
}

void AlbumWatch::Private::determineMode()
{
#ifdef USE_KNOTIFY
    if (KInotify::available())
    {
        mode = InotifyMode;
    }
    else
#endif
    {
        mode = QFSWatcherMode;
    }
}

bool AlbumWatch::Private::inBlackList(const QString& path) const
{
    // Filter out dirty signals triggered by changes on the database file
    foreach(const QString& bannedFile, fileNameBlackList)
    {
        if (path.endsWith(bannedFile))
        {
            return true;
        }
    }

    return false;
}

bool AlbumWatch::Private::inDirWatchParametersBlackList(const QFileInfo& info, const QString& path)
{
    if (params.isSQLite())
    {
        QDir dir;

        if (info.isDir())
        {
            dir = QDir(path);
        }
        else
        {
            dir = info.dir();
        }

        QFileInfo dbFile(params.SQLiteDatabaseFile());

        // Workaround for broken KDirWatch in KDE 4.2.4
        if (path.startsWith(dbFile.filePath()))
        {
            return true;
        }

        // is the signal for the directory containing the database file?
        if (dbFile.dir() == dir)
        {
            // retrieve modification dates
            QList<QDateTime> modList = buildDirectoryModList(dbFile);

            // check for equality
            if (modList == dbPathModificationDateList)
            {
                //qCDebug(DIGIKAM_GENERAL_LOG) << "Filtering out db-file-triggered dir watch signal";
                // we can skip the signal
                return true;
            }

            // set new list
            dbPathModificationDateList = modList;
        }
    }

    return false;
}

QList<QDateTime> AlbumWatch::Private::buildDirectoryModList(const QFileInfo& dbFile) const
{
    // Retrieve modification dates

    QList<QDateTime> modList;
    QFileInfoList    fileInfoList = dbFile.dir().entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    // Build the list

    foreach(const QFileInfo& info, fileInfoList)
    {
        // Ignore digikam4.db and journal and other temporary files

        if (!fileNameBlackList.contains(info.fileName()))
        {
            modList << info.lastModified();
        }
    }

    return modList;
}

// -------------------------------------------------------------------------------------

AlbumWatch::AlbumWatch(AlbumManager* const parent)
    : QObject(parent),
      d(new Private(this))
{
    d->determineMode();

#ifdef USE_KNOTIFY
    if (d->isInotifyMode())
    {
        connectToKInotify();
    }
    else
#endif
    {
        connectToQFSWatcher();
    }

    connect(parent, SIGNAL(signalAlbumAdded(Album*)),
            this, SLOT(slotAlbumAdded(Album*)));

    connect(parent, SIGNAL(signalAlbumAboutToBeDeleted(Album*)),
            this, SLOT(slotAlbumAboutToBeDeleted(Album*)));
}

AlbumWatch::~AlbumWatch()
{
    delete d;
}

void AlbumWatch::clear()
{
    if (d->dirWatch)
    {
        foreach(const QString& addedDirectory, d->dirWatchAddedDirs)
        {
            d->dirWatch->removePath(addedDirectory);
        }

        d->dirWatchAddedDirs.clear();
    }

#ifdef USE_KNOTIFY
    if (d->inotify)
    {
        d->inotify->removeAllWatches();
    }
#endif
}

void AlbumWatch::setDatabaseParameters(const DatabaseParameters& params)
{
    d->params = params;

    d->fileNameBlackList.clear();

    // filter out notifications caused by database operations
    if (params.isSQLite())
    {
        d->fileNameBlackList << QLatin1String("thumbnails-digikam.db") << QLatin1String("thumbnails-digikam.db-journal");

        QFileInfo dbFile(params.SQLiteDatabaseFile());
        d->fileNameBlackList << dbFile.fileName() << dbFile.fileName() + QLatin1String("-journal");

        // ensure this is done after setting up the black list
        d->dbPathModificationDateList = d->buildDirectoryModList(dbFile);
    }
}

void AlbumWatch::slotAlbumAdded(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* const album         = static_cast<PAlbum*>(a);
    CollectionLocation location = CollectionManager::instance()->locationForAlbumRootId(album->albumRootId());

    if (!location.isAvailable())
    {
        return;
    }

    QString dir = album->folderPath();

    if (dir.isEmpty())
    {
        return;
    }

#ifdef USE_KNOTIFY
    if (d->isInotifyMode())
    {
        d->inotify->watchDirectory(dir);
    }
    else
#endif
    {
        if (!d->dirWatch->directories().contains(dir))
        {
            d->dirWatchAddedDirs << dir;
            d->dirWatch->addPath(dir);
        }
    }
}

void AlbumWatch::slotAlbumAboutToBeDeleted(Album* a)
{
    if (a->isRoot() || a->type() != Album::PHYSICAL)
    {
        return;
    }

    PAlbum* const album = static_cast<PAlbum*>(a);
    QString dir         = album->folderPath();

    if (dir.isEmpty())
    {
        return;
    }

#ifdef USE_KNOTIFY        
    if (d->isInotifyMode())
    {
        d->inotify->removeDirectory(dir);
    }
    else
#endif
    {
        d->dirWatch->removePath(album->folderPath());
    }
}

void AlbumWatch::rescanDirectory(const QString& dir)
{
//  qCDebug(DIGIKAM_GENERAL_LOG) << "Detected change, triggering rescan of directory" << dir;
    ScanController::instance()->scheduleCollectionScanRelaxed(dir);
}

// -- KInotify ----------------------------------------------------------------------------------

#ifdef USE_KNOTIFY  
void AlbumWatch::connectToKInotify()
{
    if (d->inotify)
    {
        return;
    }

    d->inotify = new KInotify(this);

    qCDebug(DIGIKAM_GENERAL_LOG) << "AlbumWatch use KInotify";    
    
    connect( d->inotify, SIGNAL(movedFrom(QString)),
             this, SLOT(slotFileMoved(QString)) );

    connect( d->inotify, SIGNAL(movedTo(QString)),
             this, SLOT(slotFileMoved(QString)) );

/*
    connect( d->inotify, SIGNAL(moved(QString,QString)),
             this, SLOT(slotFileMoved(QString,QString)) );
*/

    connect( d->inotify, SIGNAL(deleted(QString,bool)),
             this, SLOT(slotFileDeleted(QString,bool)) );

    connect( d->inotify, SIGNAL(created(QString,bool)),
             this, SLOT(slotFileCreated(QString,bool)) );

    connect( d->inotify, SIGNAL(closedWrite(QString)),
             this, SLOT(slotFileClosedAfterWrite(QString)) );

    connect( d->inotify, SIGNAL(watchUserLimitReached()),
             this, SLOT(slotInotifyWatchUserLimitReached()) );
}

/*
// Note that moved(QString,QString) is only emitted if both source and target are watched!
// This does not apply for moving to trash, or files moved from/to non-collection directories
void AlbumWatch::slotFileMoved(const QString& from, const QString& to)
{
    // we could add a copyOrMoveHint here...but identical-file detection seems to work well
    rescanPath(from);
    rescanPath(to);
}
*/

void AlbumWatch::slotFileMoved(const QString& path)
{
    // both movedTo and movedFrom are connected to this slot
    // moved(QString,QString) is ignored, with it the information about pairing of moves
    rescanPath(path);
}

void AlbumWatch::slotFileDeleted(const QString& path, bool isDir)
{
    Q_UNUSED(isDir);
    rescanPath(path);
}

void AlbumWatch::slotFileCreated(const QString& path, bool isDir)
{
    if (isDir)
    {
        rescanPath(path);
    }
    // for files, rely on ClosedAfterWrite only,
    // which always comes after create if the operation has finished
}

void AlbumWatch::slotFileClosedAfterWrite(const QString& path)
{
    rescanPath(path);
}

void AlbumWatch::slotInotifyWatchUserLimitReached()
{
    qCDebug(DIGIKAM_GENERAL_LOG) << "Reached Inotify limit";
}

void AlbumWatch::rescanPath(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }

    QUrl url = QUrl::fromLocalFile(path);
    rescanDirectory(url.adjusted(QUrl::RemoveFilename|QUrl::StripTrailingSlash).path());
}
#endif

// -- QFileSystemWatcher ---------------------------------------------------------------------------------------

void AlbumWatch::slotQFSWatcherDirty(const QString& path)
{
    if (d->inBlackList(path))
    {
        return;
    }

    QFileInfo info(path);

    if (d->inDirWatchParametersBlackList(info, path))
    {
        return;
    }

    qCDebug(DIGIKAM_GENERAL_LOG) << "QFileSystemWatcher detected change at" << path;

    if (info.isDir())
    {
        rescanDirectory(path);
    }
    else
    {
        rescanDirectory(info.path());
    }
}

void AlbumWatch::connectToQFSWatcher()
{
    if (d->dirWatch)
    {
        return;
    }

    d->dirWatch = new QFileSystemWatcher(this);

    qCDebug(DIGIKAM_GENERAL_LOG) << "AlbumWatch use QFileSystemWatcher";

    connect(d->dirWatch, SIGNAL(directoryChanged(QString)),
            this, SLOT(slotQFSWatcherDirty(QString)));
    
    connect(d->dirWatch, SIGNAL(fileChanged(QString)),
            this, SLOT(slotQFSWatcherDirty(QString)));
}

} // namespace Digikam
