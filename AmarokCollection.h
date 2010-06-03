/*
 * This file is part of Neposync program.
 * Copyright (C) 2010 Eric Pignet <eric at erixpage dot com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef AMAROKCOLLECTION_H
#define AMAROKCOLLECTION_H

#include <QtCore/QString>
#include <QtCore/QMap>
#include <QtCore/QList>

struct st_mysql;
typedef struct st_mysql MYSQL;

class AmarokCollection
{
protected:
    MYSQL* m_db;
public:
    bool m_isVerbose;
    AmarokCollection(bool isVerbose) : m_isVerbose(isVerbose) {};
    bool connect();
    int getRating(QString url);
    bool getRating(QString iUrl, bool &oUrlPresent, int &oRating);
    bool getAllRating(QString iUrl, QMap<QString, int> &oRatings);
    bool setRating(QString iUrl, int iRating);
    bool query(QString iQuery, QList<QString> &oResult);
};

#endif // AMAROKCOLLECTION_H
