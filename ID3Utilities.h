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

#ifndef ID3UTILITIES_H
#define ID3UTILITIES_H

#include <QtCore/QString>

class ID3Utilities
{
public:
    static bool getID3Rating(QString iFileName, int &oRating, bool isVerbose = false);
    static bool setID3Rating(QString iFileName, int iRating, bool isVerbose = false);
};


#endif // ID3UTILITIES_H
