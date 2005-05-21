/* ============================================================
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2005-05-20
 * Copyright 2005 by Renchi Raju
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
 * ============================================================ */

#ifndef SEARCHRESULTSVIEW_H
#define SEARCHRESULTSVIEW_H

#include <qiconview.h>
#include <qcstring.h>
#include <qdict.h>

class QPixmap;
class KFileItem;

namespace KIO
{
class TransferJob;
class PreviewJob;
class Job;
}

class SearchResultsView : public QIconView
{
    Q_OBJECT
    
public:

    SearchResultsView(QWidget* parent);
    ~SearchResultsView();

    void openURL(const KURL& url);
    void clear();
    
private:

    KIO::TransferJob*    m_listJob;
    KIO::PreviewJob*     m_previewJob;
    QDict<QIconViewItem> m_itemDict;
    QString              m_libraryPath;
    QString              m_filter;

private slots:

    void slotData(KIO::Job *job, const QByteArray &data);
    void slotResult(KIO::Job *job);
    void slotPreview(const KFileItem* item, const QPixmap&);
    void slotFailed(const KFileItem* item);
};

#endif /* SEARCHRESULTSVIEW_H */
