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


#include <nepomuk2/resource.h>
#include <nepomuk2/resourcemanager.h>
#include <nepomuk2/tag.h>
#include <nepomuk2/variant.h>
#include <Soprano/Vocabulary/NAO>

#include <iostream>

#include <libkexiv2/kexiv2.h>

#include <QtCore/QDirIterator>

#include "AmarokCollection.h"
#include "ID3Utilities.h"

void showUsage()
{
    std::cout << "Common usage:" << std::endl;
    std::cout << "  neposync -nf [OPTIONS..] [DIRECTORY]" << std::endl;
    std::cout << "  neposync -fn [OPTIONS..] [DIRECTORY]" << std::endl;
    std::cout << "Actions (nepomuk):" << std::endl;
    std::cout << "  -nf, --nepomuk-to-files    Read tags/ratings from Nepomuk and store them in files metadata" << std::endl;
    std::cout << "  -fn, --files-to-nepomuk    Read tags/ratings from files metadata and store them in Nepomuk" << std::endl;
    std::cout << "  -dn, --display-nepomuk     Display all Nepomuk tags/ratings" << std::endl;
    std::cout << "  -cn, --clear-nepomuk       Clear all Nepomuk tags/ratings" << std::endl;
    std::cout << "Actions (amarok):" << std::endl;
    std::cout << "  -af, --amarok-to-files     Read ratings from Amarok collection and store them in files metadata" << std::endl;
    std::cout << "  -fa, --files-to-amarok     Read ratings from files metadata and store them in Amarok collection" << std::endl;
    std::cout << "  -da, --display-amarok      Display all Amarok ratings" << std::endl;
    std::cout << "  -qa, --query-amarok QUERY  Execute Mysql query QUERY in Amarok collection" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  -r   --recursive           Recurse into sub-directories" << std::endl;
    std::cout << "  -f   --force               Copy tags/ratings even if empty on source side" << std::endl;
    std::cout << "  -V   --verbose             Display all nepomuk output (depending on KDebug settings)" << std::endl;
    std::cout << "  -h   --help                Display this usage information" << std::endl;
    std::cout << "       --version             Display version and copyright information" << std::endl;
    std::cout << "DIRECTORY is optional, if absent the current directory is synchronized" << std::endl;
    std::cout << std::endl;
    std::cout << "Remarks: neposync uses IPTC 'keyword' metadata to read/store tags in image files (as Digikam)" << std::endl;
    std::cout << "         neposync uses XMP 'Rating' metadata to read/store ratings in image files (as Digikam)" << std::endl;
    std::cout << "         neposync uses ID3v2 'Popularimeter/POPM' metadata to read/store ratings in MP3 files" << std::endl;
}

void showVersion()
{
    std::cout << "Neposync 0.5" << std::endl;
    std::cout << "Copyright © 2010 Eric Pignet" << std::endl;
    std::cout << "Licence GPLv2+" << std::endl;
}

void displayFileName(QString iFileName, bool isNewFile = false, bool isVerbose = false)
{
    static bool fileDisplayed;
    if (isNewFile)
    {
        fileDisplayed = false;
    }
    if (    (isNewFile && isVerbose)
         || (!isNewFile && !fileDisplayed)
       )
    {
        std::cout << "File: " << QString(iFileName.toLocal8Bit()).toStdString() << std::endl;
        fileDisplayed = true;
    }
}


int main(int argc, char *argv[])
{
    bool isNepomukToFiles = false;
    bool isFilesToNepomuk = false;
    bool isDisplayNepomuk = false;
    bool isClearNepomuk = false;
    bool isAmarokToFiles = false;
    bool isFilesToAmarok = false;
    bool isDisplayAmarok = false;
    bool isQueryAmarok = false;
    QString amarokQuery;
    bool forceCopy = false;
    bool recurseDirectories = false;
    bool isVerbose = false;
    int nbActions = 0;
    QString workingDirectory;

    //------------------
    // Parse arguments

    for (int i=1; i<argc; ++i)
    {
        if (!strcmp(argv[i], "-nf") || !strcmp(argv[i],"--nepomuk-to-files"))
        {
            isNepomukToFiles = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-fn") || !strcmp(argv[i],"--files-to-nepomuk"))
        {
            isFilesToNepomuk = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-dn") || !strcmp(argv[i],"--display-nepomuk"))
        {
            isDisplayNepomuk = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-cn") || !strcmp(argv[i],"--clear-nepomuk"))
        {
            isClearNepomuk = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-af") || !strcmp(argv[i],"--amarok-to-files"))
        {
            isAmarokToFiles = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-fa") || !strcmp(argv[i],"--files-to-amarok"))
        {
            isFilesToAmarok = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-da") || !strcmp(argv[i],"--display-amarok"))
        {
            isDisplayAmarok = true;
            nbActions ++;
        }
        else if (!strcmp(argv[i], "-qa") || !strcmp(argv[i],"--query-amarok"))
        {
            isQueryAmarok = true;
            nbActions ++;
            i++;
            if ((i == argc) || (argv[i][0] == '-'))
            {
                std::cout << "A Mysql query must follow --query-amarok action." << std::endl;
                return 1;
            }
            else
            {
                amarokQuery = argv[i];
            }
        }
        else if (!strcmp(argv[i], "-r") || !strcmp(argv[i], "--recursive"))
        {
            recurseDirectories = true;
        }
        else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--force"))
        {
            forceCopy = true;
        }
        else if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            showUsage();
            return 0;
        }
        else if (!strcmp(argv[i], "-V") || !strcmp(argv[i], "--verbose"))
        {
            isVerbose = true;
        }
        else if (!strcmp(argv[i], "--version"))
        {
            showVersion();
            return 0;
        }
        else if (argv[i][0] != '-')
        {
            workingDirectory = argv[i];
        }
    }

    if (nbActions == 0)
    {
        showUsage();
        return 0;
    }
    else if (nbActions > 1)
    {
        std::cout << "Only one action must be specified." << std::endl;
        return 1;
    }

    //------------------
    // Initializations

    if (!isVerbose)
    {
        // Nepomuk and exiv2 libraries are *very* verbose, they pollute the program's output.
        // I didn't manage to filter them efficiently, even using KDebug settings.
        // So in "not verbose" mode, we throw all stderr to /dev/null.
        freopen ("/dev/null","w",stderr);
    }
    KExiv2Iface::KExiv2::initializeExiv2();
    Nepomuk2::ResourceManager::instance()->init();

    if (workingDirectory.isNull())
    {
        // If directory not specified, current directory is used.
        // But current directory found by Qt is not reliable: it will always return the absolute path on filesystem.
        // If files were tagged in Nepomuk via an alternative path (with symlinks), sync will not work.
        // Only pwd command can return the simplified path, if any.
        if (getenv("PWD") != NULL)
        {
            workingDirectory = QString::fromLocal8Bit(getenv("PWD"));
        }
        else
        {
            // If PWD not available on the platform, fallback to portable Qt method.
            workingDirectory = QDir::currentPath();
        }
    }
    else
    {
        // Remove final / character if present
        if (workingDirectory[workingDirectory.length()-1] == '/')
        {
            workingDirectory.truncate(workingDirectory.length()-1);
        }
    }
    if (isVerbose)
        std::cout << "Path used: " << std::string(workingDirectory.toLocal8Bit()) << std::endl;

    //------------------
    // Nepomuk to files

    if (isNepomukToFiles)
    {
        QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
        while (it.hasNext())
        {
            QString currentFileName(it.next());

            if (   !it.fileInfo().suffix().compare("jpg", Qt::CaseInsensitive)
                || !it.fileInfo().suffix().compare("jpeg", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                Nepomuk2::Resource aFile(it.filePath());

                // Copy of tags
                QList<Nepomuk2::Variant> vl = aFile.property( Soprano::Vocabulary::NAO::hasTag() ).toVariantList();
                if (!vl.isEmpty() || forceCopy)
                {
                    KExiv2Iface::KExiv2 myExifData(currentFileName);
                    QStringList oldKeywords = myExifData.getIptcKeywords();
                    QStringList newKeywords;
                    foreach (Nepomuk2::Variant vv, vl)
                    {
                        Nepomuk2::Tag tag(vv.toString());
                        newKeywords.append(tag.label());
                    }
                    QStringList oldKeywordsSorted(oldKeywords);
                    QStringList newKeywordsSorted(newKeywords);
                    oldKeywordsSorted.sort();
                    newKeywordsSorted.sort();
                    if (oldKeywordsSorted != newKeywordsSorted)
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to replace IPTC keywords to: ";
                        foreach (QString keyword, newKeywords) std::cout << keyword.toStdString() << " ";
                        std::cout << std::endl;
                        myExifData.setIptcKeywords(oldKeywords, newKeywordsSorted);
                        myExifData.applyChanges();
                    }
                }

                // Copy of rating
                if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                {
                    KExiv2Iface::KExiv2 myXMPData(currentFileName);
                    QString rating = myXMPData.getXmpTagString("Xmp.xmp.Rating");
                    if (rating.isNull() || rating.toUInt() != aFile.rating())
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to copy rating: " << QString::number(aFile.rating()).toStdString() << std::endl;
                        myXMPData.setXmpTagString("Xmp.xmp.Rating",QString::number(aFile.rating()), false);
                        myXMPData.applyChanges();
                    }
                }
                else if (forceCopy)
                {
                    KExiv2Iface::KExiv2 myXMPData(currentFileName);
                    QString rating = myXMPData.getXmpTagString("Xmp.xmp.Rating");
                    if (!rating.isNull())
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to clear rating" << std::endl;
                        myXMPData.setXmpTagString("Xmp.xmp.Rating",NULL, false);
                        myXMPData.applyChanges();
                    }
                }
            }
            else if (!it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                Nepomuk2::Resource aFile(it.filePath());

                if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                {
                    int id3rating = 0;
                    ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                    if ((unsigned int)id3rating != aFile.rating())
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to copy rating: " << aFile.rating() << "/10" << std::endl;
                        ID3Utilities::setID3Rating(currentFileName, aFile.rating(), isVerbose);
                    }
                }
                else if (forceCopy)
                {
                    int id3rating = 0;
                    ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                    if (id3rating > 0)
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to clear rating" << std::endl;
                        ID3Utilities::setID3Rating(currentFileName, 0, isVerbose);
                    }
                }
            }
        }
    }

    //------------------
    // Files to Nepomuk

    if (isFilesToNepomuk)
    {
        QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
        while (it.hasNext()) {
            QString currentFileName(it.next());

            if (   !it.fileInfo().suffix().compare("jpg", Qt::CaseInsensitive)
                || !it.fileInfo().suffix().compare("jpeg", Qt::CaseInsensitive))
            {
                KExiv2Iface::KExiv2 myExifData(currentFileName);
                displayFileName(currentFileName, true, isVerbose);

                // Copy of tags
                QStringList keywords = myExifData.getIptcKeywords();
                if (!keywords.isEmpty() || forceCopy)
                {
                    Nepomuk2::Resource aFile(it.fileInfo().absoluteFilePath());
                    QList<Nepomuk2::Tag> fileTags = aFile.tags();

                    // Remove unneeded tags, if any (more performant than removing everything then recreating)
                    QList<Nepomuk2::Variant> tagsToRemove;
                    foreach (Nepomuk2::Tag fileTag, fileTags)
                    {
                        if (!keywords.contains(fileTag.label()))
                        {
                            displayFileName(currentFileName);
                            std::cout << "  Needs to remove tag: " << fileTag.label().toStdString() << std::endl;
//@FIXME                            tagsToRemove.append(fileTag.resourceUri());
                        }
                    }
                    if (!tagsToRemove.isEmpty())
                    {
                        Nepomuk2::Variant tagsToRemoveVar(tagsToRemove);
                        aFile.removeProperty(Soprano::Vocabulary::NAO::hasTag(), tagsToRemoveVar);
                    }

                    // Add missing tags
                    foreach (const QString& keyword, keywords)
                    {
                        if (!fileTags.contains(keyword))
                        {
                            displayFileName(currentFileName);
                            std::cout << "  Needs to add tag: " << QString(keyword.toLocal8Bit()).toStdString() << std::endl;
                            Nepomuk2::Tag tag(keyword);
                            tag.setLabel(keyword);
                            aFile.addTag(tag);
                        }
                    }
                }

                // Copy of rating
                QString rating = myExifData.getXmpTagString("Xmp.xmp.Rating");
                if (!rating.isNull())
                {
                    Nepomuk2::Resource aFile(it.fileInfo().absoluteFilePath());
                    if (!aFile.hasProperty(Soprano::Vocabulary::NAO::rating()) || rating.toUInt() != aFile.rating())
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to replace rating: " << rating.toStdString() << std::endl;
                        aFile.setRating(rating.toUInt());
                    }
                }
                else if (forceCopy)
                {
                    Nepomuk2::Resource aFile(it.fileInfo().absoluteFilePath());
                    if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to clear rating" << std::endl;
                        aFile.removeProperty(Soprano::Vocabulary::NAO::rating());
                    }
                }
            }
            else if (!it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                int id3rating = 0;
                ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                if (id3rating > 0)
                {
                    Nepomuk2::Resource aFile(it.fileInfo().absoluteFilePath());
                    if (!aFile.hasProperty(Soprano::Vocabulary::NAO::rating()) || ((unsigned int)id3rating != aFile.rating()))
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to replace rating: " << id3rating << std::endl;
                        aFile.setRating((unsigned int)id3rating);
                    }
                }
                else if (forceCopy)
                {
                    Nepomuk2::Resource aFile(it.fileInfo().absoluteFilePath());
                    if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                    {
                        displayFileName(currentFileName);
                        std::cout << "  Needs to clear rating" << std::endl;
                        aFile.removeProperty(Soprano::Vocabulary::NAO::rating());
                    }
                }
            }
        }
    }

    //------------------
    // Display Nepomuk

    if (isDisplayNepomuk)
    {
        if (forceCopy)
        {
            std::cout << "In display-nepomuk mode, --force-copy option has no effect." << std::endl;
        }

        QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
        while (it.hasNext())
        {
            QString currentFileName(it.next());

            if (   !it.fileInfo().suffix().compare("jpg", Qt::CaseInsensitive)
                || !it.fileInfo().suffix().compare("jpeg", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                Nepomuk2::Resource aFile(it.filePath());

                // Display tags
                QList<Nepomuk2::Tag> fileTags = aFile.tags();
                if (!fileTags.isEmpty())
                {
                    displayFileName(currentFileName);
                    std::cout << "  Tags:";
                    foreach (Nepomuk2::Tag fileTag, fileTags)
                    {
                        std::cout << " " << QString(fileTag.label().toLocal8Bit()).toStdString();
                    }
                    std::cout << std::endl;
                }

                // Display rating
                if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                {
                    displayFileName(currentFileName);
                    std::cout << "  Rating: " << aFile.rating() << std::endl;
                }
            }
            else if (!it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                Nepomuk2::Resource aFile(it.filePath());
                if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                {
                    displayFileName(currentFileName);
                    std::cout << "  Rating: " << aFile.rating() << std::endl;
                }
            }
        }
    }

    //------------------
    // Clear Nepomuk

    if (isClearNepomuk)
    {
        if (forceCopy)
        {
            std::cout << "In clear-nepomuk mode, --force-copy option has no effect." << std::endl;
        }

        QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
        while (it.hasNext())
        {
            QString currentFileName(it.next());

            if (   !it.fileInfo().suffix().compare("jpg", Qt::CaseInsensitive)
                || !it.fileInfo().suffix().compare("jpeg", Qt::CaseInsensitive)
                || !it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
            {
                displayFileName(currentFileName, true, isVerbose);

                Nepomuk2::Resource aFile(it.filePath());

                // Clear tags
                QList<Nepomuk2::Tag> fileTags = aFile.tags();
                if (!fileTags.isEmpty())
                {
                    displayFileName(currentFileName);
                    std::cout << "  Remove tags:";
                    foreach (Nepomuk2::Tag fileTag, fileTags)
                    {
                        std::cout << " " << fileTag.label().toStdString();
//@FIXME                        aFile.removeProperty(Soprano::Vocabulary::NAO::hasTag(), fileTag.resourceUri());
                    }
                    std::cout << std::endl;
                }

                // Clear rating
                if (aFile.hasProperty(Soprano::Vocabulary::NAO::rating()))
                {
                    displayFileName(currentFileName);
                    std::cout << "  Clear rating" << std::endl;
                    aFile.removeProperty(Soprano::Vocabulary::NAO::rating());
                }
            }
        }
    }

    // Amarok initialization
    if (isAmarokToFiles || isFilesToAmarok || isDisplayAmarok || isQueryAmarok)
    {
        AmarokCollection amarokDb(isVerbose);
        if (!amarokDb.connect())
        {
            return 1;
        }

        //------------------
        // Amarok to Files

        if (isAmarokToFiles)
        {
            QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
            while (it.hasNext())
            {
                QString currentFileName(it.next());

                if (!it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
                {
                    displayFileName(currentFileName, true, isVerbose);

                    bool urlPresent = false;
                    int amarokRating = 0;
                    amarokDb.getRating(currentFileName, urlPresent, amarokRating);
                    if (amarokRating > 0)
                    {
                        int id3rating = 0;
                        ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                        if (id3rating != amarokRating)
                        {
                            displayFileName(currentFileName);
                            std::cout << "  Needs to copy rating: " << amarokRating << "/10" << std::endl;
                            ID3Utilities::setID3Rating(currentFileName, amarokRating, isVerbose);
                        }
                    }
                    else if (forceCopy)
                    {
                        int id3rating = 0;
                        ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                        if (id3rating != 0)
                        {
                            displayFileName(currentFileName);
                            std::cout << "  Needs to clear rating" << std::endl;
                            ID3Utilities::setID3Rating(currentFileName, 0, isVerbose);
                        }
                    }
                }
            }
        }

        //------------------
        // Files to Amarok

        if (isFilesToAmarok)
        {
            QDirIterator it(workingDirectory, QDir::Files | QDir::NoDotAndDotDot, (recurseDirectories?QDirIterator::Subdirectories:QDirIterator::NoIteratorFlags));
            while (it.hasNext())
            {
                QString currentFileName(it.next());

                if (!it.fileInfo().suffix().compare("mp3", Qt::CaseInsensitive))
                {
                    displayFileName(currentFileName, true, isVerbose);

                    int id3rating = 0;
                    ID3Utilities::getID3Rating(currentFileName, id3rating, isVerbose);
                    if (id3rating > 0)
                    {
                        bool urlPresent = false;
                        int amarokRating = 0;
                        amarokDb.getRating(currentFileName, urlPresent, amarokRating);
                        if (!urlPresent)
                        {
                            displayFileName(currentFileName);
                            std::cout << "  File has rating " << id3rating << " but is not in Amarok collection. Do nothing" << std::endl;
                        }
                        else
                        {
                            if (id3rating != amarokRating)
                            {
                                displayFileName(currentFileName);
                                std::cout << "  Needs to copy rating: " << id3rating << std::endl;
                                amarokDb.setRating(currentFileName, id3rating);
                            }
                        }
                    }
                    else if (forceCopy)
                    {
                        bool urlPresent = false;
                        int amarokRating = 0;
                        amarokDb.getRating(currentFileName, urlPresent, amarokRating);
                        if (amarokRating != 0)
                        {
                            displayFileName(currentFileName);
                            std::cout << "  Needs to clear rating" << std::endl;
                            amarokDb.setRating(currentFileName, id3rating);
                        }
                    }
                }
            }
        }

        //------------------
        // Display Amarok

        if (isDisplayAmarok)
        {
            QMap<QString, int> ratings;
            amarokDb.getAllRating(workingDirectory, ratings);

            QMap<QString, int>::const_iterator i = ratings.constBegin();
            while (i != ratings.constEnd())
            {
                if (recurseDirectories ||
                    (i.key().count("/") == workingDirectory.count("/")+1))
                {
                    std::cout << QString(i.key().toLocal8Bit()).toStdString() << ": " << i.value() << std::endl;
                }
                ++i;
            }
        }

        //------------------
        // Query Amarok

        if (isQueryAmarok)
        {
            if (isVerbose)
            {
                std::cout << "Query: " << amarokQuery.toStdString() << std::endl;
            }

            QList<QString> rows;
            amarokDb.query(amarokQuery, rows);

            QList<QString>::const_iterator row = rows.constBegin();
            while (row != rows.constEnd())
            {
                std::cout << (*row).toStdString() << std::endl;
                ++row;
            }
        }
    }

    if (!isVerbose)
    {
        fclose(stderr);
    }

    return 0;
}
