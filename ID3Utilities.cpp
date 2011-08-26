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


//#include <taglib/fileref.h>
#include <taglib/mpegfile.h>
#include <taglib/id3v2tag.h>
#include <taglib/popularimeterframe.h>

#include <iostream>

#include "ID3Utilities.h"

bool ID3Utilities::getID3Rating(QString iFileName, int &oRating, bool isVerbose)
{
    oRating = 0;
    TagLib::MPEG::File file(QString(iFileName.toLocal8Bit()).toStdString().c_str());
    if (file.ID3v2Tag())
    {
        TagLib::ID3v2::FrameList l = file.ID3v2Tag()->frameListMap()["POPM"];
        if (!l.isEmpty())
        {
            if (isVerbose)
                std::cout << "  Full POPM frame: " << l.front()->toString() << std::endl;
            TagLib::ID3v2::PopularimeterFrame *popFrame = new TagLib::ID3v2::PopularimeterFrame(l.front()->render());
            if (popFrame != 0)
            {
                oRating = popFrame->rating();
                if (isVerbose)
                    std::cout << "  Convert " << oRating << "/255 to " << qRound(qreal(oRating) *10/255) << "/10" << std::endl;
                oRating = qRound(qreal(oRating)*10/255);
            }
        }
    }
    return true;
}


bool ID3Utilities::setID3Rating(QString iFileName, int iRating, bool isVerbose)
{
    TagLib::MPEG::File f(QString(iFileName.toLocal8Bit()).toStdString().c_str());
    // Check to make sure that it has an ID3v2 tag
    // TODO add ID3v2 tag if needed
    if(f.ID3v2Tag())
    {
        f.ID3v2Tag()->removeFrames("POPM");

        TagLib::ID3v2::PopularimeterFrame *popFrame = new TagLib::ID3v2::PopularimeterFrame();
        if (popFrame != 0)
        {
            if (isVerbose)
                std::cout << "  Convert " << iRating << "/10 to " << qRound(qreal(iRating) * 255 / 10) << "/255" << std::endl;
            popFrame->setRating(qRound(qreal(iRating) * 255 / 10));
            f.ID3v2Tag()->addFrame(popFrame);
            if (!f.save())
            {
                std::cout << "Cannot save file" << std::endl;
                return false;
            }
        }
        else
        {
            std::cout << "Cannot create ID3v2 frame" << std::endl;
            return false;
        }
    }
    else
    {
        std::cout << "This file has no ID3v2 tag. No ID3v2 creation in Neposync so far" << std::endl;
        return false;
    }
    return true;
}


