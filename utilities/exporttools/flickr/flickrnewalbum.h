/* ============================================================
 *
 * This file is a part of digiKam project
 * http://www.digikam.org
 *
 * Date        : 2015-08-07
 * Description : a dialog to create a new remote album for export tools
 *
 * Copyright (C) 2013 by Pankaj Kumar <me at panks dot me>
 * Copyright (C) 2015 by Shourya Singh Gupta <shouryasgupta at gmail dot com>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#ifndef FLICKR_NEW_ALBUM_H
#define FLICKR_NEW_ALBUM_H

// Local includes

#include "newalbumdialog.h"

namespace Digikam
{

class FPhotoSet;

class FlickrNewAlbum : public NewAlbumDialog
{
    Q_OBJECT

public:

    explicit FlickrNewAlbum(QWidget* const parent, const QString& pluginName);
    ~FlickrNewAlbum();

    void getFolderProperties(FPhotoSet& folder);
};

} // namespace Digikam

#endif // FLICKR_NEW_ALBUM_H