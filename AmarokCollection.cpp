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


/*
 * Unfortunately QtSql cannot connect to an embedded Mysql file (mysqle).
 * A solution could be to write a QtSql driver for Mysqle.
 * But for neposync we'll use directly mysql API.
 * Part of this file is extracted from MySqlEmbeddedStorage.cpp (Amarok sources, GPL licence).
 */

#include <mysql/mysql.h>
#include "AmarokCollection.h"

#include <iostream>

#include <kstandarddirs.h>
#include <kglobal.h>
#include <QtCore/QString>
#include <QtCore/QDir>

bool AmarokCollection::connect()
{
    QString defaultsFile;

    // TODO improve amarok directory detection (mimic amarok itself ?)
    QString storageLocation = KGlobal::dirs()->localkdedir() + "/share/apps/amarok/";
    QString databaseDir;
    QDir dir( storageLocation);
    dir.mkpath( "." );  //ensure directory exists
    defaultsFile = QDir::cleanPath( dir.absoluteFilePath( "my.cnf" ) );
    databaseDir = dir.absolutePath() + QDir::separator() + "mysqle";
    if (m_isVerbose)
    {
        std::cout << "Path of Amarok mysqle database: " << databaseDir.toStdString() << std::endl;
    }

    char* defaultsLine = qstrdup( QString( "--defaults-file=%1" ).arg( defaultsFile ).toAscii().data() );
    char* databaseLine = qstrdup( QString( "--datadir=%1" ).arg( databaseDir ).toAscii().data() );

    if( !QFile::exists( defaultsFile ) )
    {
        QFile df( defaultsFile );
        if ( !df.open( QIODevice::WriteOnly ) )
        {
            std::cout << "Error: Unable to open " << defaultsFile.toStdString() << " for writing." << std::endl;
            return false;
        }
    }

    if( !QFile::exists( databaseDir ) )
    {
        std::cout << "Error: mysql database does not exist: " << databaseDir.toStdString() << std::endl;
        return false;
    }

    static const int num_elements = 9;
    char **server_options = new char* [ num_elements + 1 ];
    server_options[0] = const_cast<char*>( "amarokmysqld" );
    server_options[1] = defaultsLine;
    server_options[2] = databaseLine;
    // CAUTION: if we ever change the table type we will need to fix a number of MYISAM specific
    // functions, such as FULLTEXT indexing.
    server_options[3] = const_cast<char*>( "--default-storage-engine=MYISAM" );
    server_options[4] = const_cast<char*>( "--loose-skip-innodb" );
    server_options[5] = const_cast<char*>( "--skip-grant-tables" );
    server_options[6] = const_cast<char*>( "--myisam-recover=FORCE" );
    server_options[7] = const_cast<char*>( "--character-set-server=utf8" );
    server_options[8] = const_cast<char*>( "--collation-server=utf8_bin" );
    server_options[num_elements] = 0;

    char **server_groups = new char* [ 3 ];
    server_groups[0] = const_cast<char*>( "amarokserver" );
    server_groups[1] = const_cast<char*>( "amarokclient" );
    server_groups[2] = 0;

    if( mysql_library_init(num_elements, server_options, server_groups) != 0 )
    {
        std::cout << "MySQL library initialization failed." << std::endl;
        return false;
    }

    m_db = mysql_init( NULL );
    delete [] server_options;
    delete [] server_groups;
    delete [] defaultsLine;
    delete [] databaseLine;

    if( !m_db )
    {
        std::cout << "Error: MySQLe initialization failed" << std::endl;
        return false;
    }

    if( mysql_options( m_db, MYSQL_READ_DEFAULT_GROUP, "amarokclient" ) )
        std::cout << "Error setting options for READ_DEFAULT_GROUP" << std::endl;
    if( mysql_options( m_db, MYSQL_OPT_USE_EMBEDDED_CONNECTION, NULL ) )
        std::cout << "Error setting option to use embedded connection" << std::endl;

    if( !mysql_real_connect( m_db, NULL,NULL,NULL, "amarok", 0,NULL, 0 ) )
    {
        std::cout << "Could not connect to mysql!" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        mysql_close( m_db );
        m_db = 0;
        return false;
    }

    if (m_isVerbose)
    {
        std::cout << "Connected to MySQLe server" << mysql_get_server_info( m_db ) << std::endl;
    }
    return true;
}

bool AmarokCollection::getRating(QString iUrl, bool &oUrlPresent, int &oRating)
{
    MYSQL_RES *result;
    MYSQL_ROW row;
    oUrlPresent = false;
    oRating = 0;

    //std::string query("select rating from statistics s, urls u where s.url=u.id and u.rpath='" + iUrl.toStdString() + "'");
    std::string query("SELECT s.rating FROM urls u LEFT OUTER JOIN statistics s ON s.url=u.id WHERE u.rpath='" + QString(iUrl.toLocal8Bit()).toStdString() + "'");
    if (mysql_query(m_db, query.c_str()) != 0)
    {
        std::cout << "Error in Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }
    if (!(result = mysql_store_result(m_db)))
    {
        std::cout << "Error in storing results of Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }

    if ((row = mysql_fetch_row(result)) != 0)
    {
        oUrlPresent = true;
        if (row[0] != NULL)
        {
            QString s_rating(row[0]);
            oRating = s_rating.toInt();
        }
        // else, no rating for this URL
    }
    // else, no URL

    mysql_free_result(result);
    return true;
}

bool AmarokCollection::getAllRating(QString iUrl, QMap<QString, int> &oRatings)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    std::string query("select rpath, rating from statistics s, urls u where s.url=u.id and u.rpath like '" + QString(iUrl.toLocal8Bit()).toStdString() + "%'");
    if (mysql_query(m_db, query.c_str()) != 0)
    {
        std::cout << "Error in Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }
    if (!(result = mysql_store_result(m_db)))
    {
        std::cout << "Error in storing results of Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }

    while ((row = mysql_fetch_row(result)) != 0)
    {
        int rating = QString(row[1]).toInt();
        if (rating > 0)
        {
           oRatings[QString(row[0])] = rating;
        }
    }
    // else, no rating for this URL

    mysql_free_result(result);
    return true;
}

// Prerequisite to call this method: iUrl is present in Amarok collection
// (please test with getRating before)
bool AmarokCollection::setRating(QString iUrl, int iRating)
{
    MYSQL_RES *result;
    MYSQL_ROW row;

    std::string query("SELECT u.id from urls u LEFT OUTER JOIN statistics s ON s.url=u.id WHERE u.rpath='" + QString(iUrl.toLocal8Bit()).toStdString() + "'");
    if (mysql_query(m_db, query.c_str()) != 0)
    {
        std::cout << "Error in Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }
    if (!(result = mysql_store_result(m_db)))
    {
        std::cout << "Error in storing results of Mysqle query to retrieve rating from url" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }
    if ((row = mysql_fetch_row(result)) == 0)
    {
        std::cout << "Error: method setRating should be called only if Url is present in Amarok collection" << std::endl;
        return false;
    }
    else
    {
        if (row[1] == NULL)
        {
            //---------------------------------------
            // There is no statistic row in database

            QString queryInsert("INSERT INTO statistics(url, rating) VALUES('%1', '%2')");
            queryInsert = queryInsert.arg(QString(row[0]), QString::number(iRating));
            //std::cout << queryInsert.toStdString() << std::endl;
            if (mysql_query(m_db, queryInsert.toStdString().c_str()) != 0)
            {
                std::cout << "Error in Mysqle query to insert rating" << std::endl;
                std::cout << "Error: " << mysql_error(m_db) << std::endl;
                return false;
            }
        }
        else
        {
            //----------------------------------------------
            // There is already a statistic row in database

            QString queryUpdate("UPDATE statistics SET rating='%1' WHERE url='%2'");
            queryUpdate = queryUpdate.arg(QString::number(iRating), QString(row[0]));
            //std::cout << queryUpdate.toStdString() << std::endl;
            if (mysql_query(m_db, queryUpdate.toStdString().c_str()) != 0)
            {
                std::cout << "Error in Mysqle query to update rating" << std::endl;
                std::cout << "Error: " << mysql_error(m_db) << std::endl;
                return false;
            }
        }
    }
    return true;
}


bool AmarokCollection::query(QString iQuery, QList<QString> &oResult)
{
    MYSQL_RES *result;
    MYSQL_FIELD *fields;
    MYSQL_ROW row;
    unsigned int num_fields = 0;

    if (mysql_query(m_db, iQuery.toLocal8Bit()) != 0)
    {
        std::cout << "Error in Mysqle query" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }
    if (!(result = mysql_store_result(m_db)))
    {
        std::cout << "Error in storing results of Mysqle query" << std::endl;
        std::cout << "Error: " << mysql_error(m_db) << std::endl;
        return false;
    }

    num_fields = mysql_num_fields(result);
    fields = mysql_fetch_fields(result);
    QString headers;
    for (unsigned int i=0; i<num_fields; i++)
    {
        if (i>0)
            headers += ",";
        headers += fields[i].name;
    }
    oResult.push_back(headers);
    while ((row = mysql_fetch_row(result)) != 0)
    {
        QString row_s;
        for (unsigned int i=0; i<num_fields; i++)
        {
            if (i>0)
                row_s += ",";
            row_s += row[i];
        }
        oResult.push_back(row_s);
    }

    mysql_free_result(result);
    return true;
}
