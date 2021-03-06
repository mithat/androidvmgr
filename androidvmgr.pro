#
#  Copyright 2013 Mithat Konar
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
#  MA 02110-1301, USA.
#
#-------------------------------------------------
#
# Project created by QtCreator 2013-07-24T23:27:42
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = androidvmgr
TEMPLATE = app

# from: http://www.qtcentre.org/wiki/index.php?title=Version_numbering_using_QMake
VERSION = 0.2.2
VERSTR = '\\"$${VERSION}\\"'  # place quotes around the version string
DEFINES += VER=\"$${VERSTR}\" # create a VER macro containing the version string

SOURCES += main.cpp\
        mainwindow.cpp \
    preferencesdialog.cpp

HEADERS  += mainwindow.h \
    preferencesdialog.h

FORMS    += mainwindow.ui \
    preferencesdialog.ui

RESOURCES += \
    icons.qrc
