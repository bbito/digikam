/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2006-12-26
 * Description : Multithreaded loader for previews
 *
 * Copyright (C) 2006-2008 by Marcel Wiesweg <marcel.wiesweg@gmx.de>
 * Copyright (C) 2006-2008 by Gilles Caulier <caulier dot gilles at gmail dot com>
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

#include "previewtask.h"

// C ANSI includes

#include <cmath>

// Qt includes

#include <QApplication>
#include <QImage>
#include <QVariant>
#include <QMatrix>

// KDE includes

#include <kdebug.h>

// LibKDcraw includes

#include <libkdcraw/kdcraw.h>

// Local includes

#include "dmetadata.h"
#include "jpegutils.h"
#include "previewloadthread.h"

namespace Digikam
{

void PreviewLoadingTask::execute()
{
    if (m_loadingTaskStatus == LoadingTaskStatusStopping)
        return;

    LoadingCache *cache = LoadingCache::cache();
    {
        LoadingCache::CacheLock lock(cache);

        // find possible cached images
        DImg *cachedImg = 0;
        QStringList lookupKeys = m_loadingDescription.lookupCacheKeys();
        // lookupCacheKeys returns "best first". Prepend the cache key to make the list "fastest first":
        // Scaling a full version takes longer!
        lookupKeys.push_front(m_loadingDescription.cacheKey());
        for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
            if ( (cachedImg = cache->retrieveImage(*it)) )
                break;
        }

        if (cachedImg)
        {
            // image is found in image cache, loading is successful

            m_img = *cachedImg;
            if (accessMode() == LoadSaveThread::AccessModeReadWrite)
                m_img = m_img.copy();

            // rotate if needed - images are unrotated in the cache,
            // except for RAW images, which are already rotated by dcraw.
            if (m_loadingDescription.previewParameters.exifRotate)
            {
                m_img = m_img.copy();
                LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);
            }
        }
        else
        {
            // find possible running loading process
            m_usedProcess = 0;
            for ( QStringList::Iterator it = lookupKeys.begin(); it != lookupKeys.end(); ++it ) {
                if ( (m_usedProcess = cache->retrieveLoadingProcess(*it)) )
                {
                    break;
                }
            }
            // do not wait on other loading processes?
            //m_usedProcess = cache->retrieveLoadingProcess(m_loadingDescription.cacheKey());

            if (m_usedProcess)
            {
                // Other process is right now loading this image.
                // Add this task to the list of listeners and
                // attach this thread to the other thread, wait until loading
                // has finished.
                m_usedProcess->addListener(this);
                // break loop when either the loading has completed, or this task is being stopped
                while ( m_loadingTaskStatus != LoadingTaskStatusStopping && m_usedProcess && !m_usedProcess->completed())
                    lock.timedWait();
                // remove listener from process
                if (m_usedProcess)
                    m_usedProcess->removeListener(this);
                // wake up the process which is waiting until all listeners have removed themselves
                lock.wakeAll();
                // set to 0, as checked in setStatus
                m_usedProcess = 0;
                // m_img is now set to the result
            }
            else
            {
                // Neither in cache, nor currently loading in different thread.
                // Load it here and now, add this LoadingProcess to cache list.
                cache->addLoadingProcess(this);
                // Add this to the list of listeners
                addListener(this);
                // for use in setStatus
                m_usedProcess = this;
                // Notify other processes that we are now loading this image.
                // They might be interested - see notifyNewLoadingProcess below
                cache->notifyNewLoadingProcess(this, m_loadingDescription);
            }
        }
    }

    if (!m_img.isNull())
    {
        // following the golden rule to avoid deadlocks, do this when CacheLock is not held

        // The image from the cache may or may not be post processed.
        // postProcess() will detect if work is needed.
        postProcess();
        m_thread->taskHasFinished();
        m_thread->imageLoaded(m_resultLoadingDescription, m_img);
        return;
    }

    // load image
    int  size = m_loadingDescription.previewParameters.size;

    QImage qimage;
    bool fromEmbeddedPreview = false;

    // -- Get the image preview --------------------------------

    if (size)
    {
        // First the QImage-dependent loading methods
        // Trying to load with dcraw: RAW files.
        if (KDcrawIface::KDcraw::loadEmbeddedPreview(qimage, m_loadingDescription.filePath))
        {
            // Discard if too small
            if (qMax(qimage.width(), qimage.height()) < size / 2)
                qimage = QImage();
            else
                fromEmbeddedPreview = true;
        }

        if (qimage.isNull())
        {
            //TODO: Use DImg based loader instead?
            KDcrawIface::KDcraw::loadHalfPreview(qimage, m_loadingDescription.filePath);
        }

        // Try to extract Exif/IPTC preview.
        if (qimage.isNull())
        {
            loadImagePreview(qimage, m_loadingDescription.filePath);
        }

        if (!qimage.isNull())
        {
            // convert from QImage
            m_img = DImg(qimage);
            // mark as embedded preview (for Exif rotation)
            if (fromEmbeddedPreview)
                m_img.setAttribute("fromRawEmbeddedPreview", true);
            // free memory
            qimage = QImage();
        }

        // DImg-dependent loading methods
        if (m_img.isNull())
        {
            // Set a hint to try to load a JPEG or PGF with the fast scale-before-decoding method
            m_img.setAttribute("scaledLoadingSize", size);
            m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
        }

        if (m_img.isNull())
        {
            kWarning(50003) << "Cannot extract preview for " << m_loadingDescription.filePath;
        }
    }
    else
    {
        //TODO: Use Exiv2's PreviewManager
        KDcrawIface::DcrawInfoContainer dcrawIdentify;
        if (KDcrawIface::KDcraw::rawFileIdentify(dcrawIdentify, m_loadingDescription.filePath)
            && dcrawIdentify.isDecodable)
        {
            // Trying to load with dcraw: RAW files.
            if (KDcrawIface::KDcraw::loadEmbeddedPreview(qimage, m_loadingDescription.filePath))
            {
                // discard if smaller than half preview
                if (qimage.width() < dcrawIdentify.imageSize.width() / 2)
                    qimage = QImage();
                else
                    fromEmbeddedPreview = true;
            }

            if (qimage.isNull())
            {
                KDcrawIface::KDcraw::loadHalfPreview(qimage, m_loadingDescription.filePath);
            }
        }

        if (!qimage.isNull())
        {
            // convert from QImage
            m_img = DImg(qimage);
            // mark as embedded preview (for Exif rotation)
            if (fromEmbeddedPreview)
            {
                m_img.setAttribute("fromRawEmbeddedPreview", true);
                /*
                DMetadata metadata(m_loadingDescription.filePath);
                if (metaData.getImageColorWorkSpace() == DMetadata::WORKSPACE_ADOBERGB)
                    m_img.setIccProfile(IccProfile::adobeRGB());
                */
            }
            // free memory
            qimage = QImage();
        }

        // DImg-dependent loading methods
        if (m_img.isNull())
        {
            m_img.load(m_loadingDescription.filePath, this, m_loadingDescription.rawDecodingSettings);
        }

        if (m_img.isNull())
        {
            kWarning(50003) << "Cannot extract preview for " << m_loadingDescription.filePath;
        }
    }

    m_img.convertToEightBit();

    // Reduce size of image:
    // - only scale down if size is considerably larger
    // - only scale down, do not scale up
    QSize scaledSize = m_img.size();
    if (needToScale(scaledSize, size))
    {
        scaledSize.scale(size, size, Qt::KeepAspectRatio);
        m_img = m_img.smoothScale(scaledSize.width(), scaledSize.height());
    }

    // Scale if hinted, Store previews rotated in the cache (?)
    if (m_loadingDescription.previewParameters.exifRotate)
        LoadSaveThread::exifRotate(m_img, m_loadingDescription.filePath);

    // For previews, we put the image post processed in the cache
    postProcess();

    {
        LoadingCache::CacheLock lock(cache);
        // put (valid) image into cache of loaded images
        if (!m_img.isNull())
            cache->putImage(m_loadingDescription.cacheKey(), new DImg(m_img), m_loadingDescription.filePath);
        // remove this from the list of loading processes in cache
        cache->removeLoadingProcess(this);
    }

    {
        LoadingCache::CacheLock lock(cache);
        // indicate that loading has finished so that listeners can stop waiting
        m_completed = true;

        // dispatch image to all listeners, including this
        for (int i=0; i<m_listeners.count(); ++i)
        {
            LoadingProcessListener *l = m_listeners[i];
            if (l->accessMode() == LoadSaveThread::AccessModeReadWrite)
            {
                // If a listener requested ReadWrite access, it gets a deep copy.
                // DImg is explicitly shared.
                DImg copy = m_img.copy();
                l->setResult(m_loadingDescription, copy);
            }
            else
            {
                l->setResult(m_loadingDescription, m_img);
            }
        }

        for (int i=0; i<m_listeners.count(); ++i)
        {
            m_listeners[i]->loadSaveNotifier()->imageLoaded(m_loadingDescription, m_img);
        }

        // remove myself from list of listeners
        removeListener(this);
        // wake all listeners waiting on cache condVar, so that they remove themselves
        lock.wakeAll();
        // wait until all listeners have removed themselves
        while (m_listeners.count() != 0)
            lock.timedWait();
        // set to 0, as checked in setStatus
        m_usedProcess = 0;
    }

    // again: following the golden rule to avoid deadlocks, do this when CacheLock is not held
    m_thread->taskHasFinished();
    m_thread->imageLoaded(m_loadingDescription, m_img);
}

bool PreviewLoadingTask::needToScale(const QSize& imageSize, int previewSize)
{
    if (!previewSize)
        return false;
    int maxSize = imageSize.width() > imageSize.height() ? imageSize.width() : imageSize.height();
    int acceptableUpperSize = lround(1.25 * (double)previewSize);
    return  maxSize >= acceptableUpperSize;
}

// -- Exif/IPTC preview extraction using Exiv2 --------------------------------------------------------

bool PreviewLoadingTask::loadImagePreview(QImage& image, const QString& path)
{
    DMetadata metadata(path);
    if (metadata.getImagePreview(image))
    {
        kDebug(50003) << "Use Exif/IPTC preview extraction. Size of image: "
                      << image.width() << "x" << image.height();
        return true;
    }

    return false;
}

} // namespace Digikam
