/* ============================================================
 * File  : splashscreen.cpp
 * Author: Renchi Raju <renchi@pooh.tam.uiuc.edu>
 * Date  : 2003-02-10
 * Description :
 *
 * Copyright 2003 by Renchi Raju

 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published bythe Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include <qpixmap.h>
#include <qstring.h>
#include <qapplication.h>
#include <qtimer.h>

#include <klocale.h>
#include <kstandarddirs.h>

#include "splashscreen.h"


SplashScreen::SplashScreen()
//    : QWidget( 0, 0, WStyle_Customize | WStyle_Splash )
    : QWidget(0, 0, WStyle_Customize|WStyle_NoBorder|WStyle_StaysOnTop|WX11BypassWM)
{
    QString file = locate( "appdata", "digikam-splash.png" );

    pix_ = new QPixmap(file);

    setErasePixmap( *pix_ );
    resize( pix_->size() );
    QRect scr = QApplication::desktop()->screenGeometry();
    move( scr.center() - rect().center() );
    show();
    repaint();

    close_ = false;
    
    timer_ = new QTimer;
    connect(timer_, SIGNAL(timeout()),
            this,   SLOT(slotClose()));
    timer_->start(1000, true);
}

SplashScreen::~SplashScreen()
{
    delete pix_;
    delete timer_;
}

#if defined(Q_WS_X11)
void qt_wait_for_window_manager( QWidget *widget );
#endif

void SplashScreen::finish( QWidget *mainWin )
{
#if defined(Q_WS_X11)
    qt_wait_for_window_manager( mainWin );
#endif
    close_ = true;
    slotClose();
}

void SplashScreen::repaint()
{
    QWidget::repaint();
    QApplication::flush();
}

void SplashScreen::mousePressEvent( QMouseEvent * )
{
    hide();
}

void SplashScreen::slotClose()
{
    if (!close_) {
        timer_->start(500, true);
        return;
    }
    
    if (timer_->isActive()) return;
    delete this;
}

#include "splashscreen.moc"
