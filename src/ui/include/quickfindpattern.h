/*
 * Copyright (C) 2010, 2013 Nicolas Bonnefon and other contributors
 *
 * This file is part of glogg.
 *
 * glogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * glogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with glogg.  If not, see <http://www.gnu.org/licenses/>.
 */

/*
 * Copyright (C) 2016 -- 2019 Anton Filimonov and other contributors
 *
 * This file is part of klogg.
 *
 * klogg is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * klogg is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with klogg.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef QUICKFINDPATTERN_H
#define QUICKFINDPATTERN_H

#include <QList>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include "common.h"

class QuickFind;

static std::vector<QString> listPreDefine = {
	QStringLiteral( "info" ), 
	QStringLiteral( "warning" ) 
};

// Represents a match result for QuickFind
class QuickFindMatch {
  public:
    //enum ChunkType = Common::ChunkType;
    // Construct a match (must be initialised)
    QuickFindMatch( int start_column, int length )
    {
        startColumn_ = start_column;
        length_ = length;
        chunkType_ = Common::ChunkType::Highlighted;
    }

    QuickFindMatch( int start_column, int length, Common::ChunkType type )
    {
        startColumn_ = start_column;
        length_ = length;
        chunkType_ = type;
    }

    // Accessor functions
    int startColumn() const
    {
        return startColumn_;
    }
    int length() const
    {
        return length_;
    }

    Common::ChunkType chunkType() const
    {
        return chunkType_;
    }

  private:
    int startColumn_;
    int length_;
    Common::ChunkType chunkType_;
};

class QuickFindMatcher {
  public:
    QuickFindMatcher()
    {}

    QuickFindMatcher( bool isActive, QRegularExpression regexp )
        : isActive_( isActive )
        , regexp_{ std::move( regexp ) }
    {
    }

    bool isActive() const
    {
        return isActive_;
    }

    // Returns whether there is a match in the passed line, starting at
    // the passed column.
    // Results are stored internally.
    bool isLineMatching( const QString& line, int column = 0 ) const;

    // Same as isLineMatching but search backward
    bool isLineMatchingBackward( const QString& line, int column = -1 ) const;

    // Must be called when isLineMatching returns 'true', returns
    // the position of the first match found.
    void getLastMatch( int* start_col, int* end_col ) const;

  private:
    bool isActive_ = false;
    QRegularExpression regexp_;

    mutable int lastMatchStart_ = 0;
    mutable int lastMatchEnd_ = 0;
};

// Represents a search pattern for QuickFind (without its results)
class QuickFindPattern : public QObject {
    Q_OBJECT

  public:
    // Construct an empty search
    QuickFindPattern() = default;

    // Set the search to a new pattern, using the current
    // case status
    void changeSearchPattern( const QString& pattern );

    // Set the search to a new pattern, as well as the case status
    void changeSearchPattern( const QString& pattern, bool ignoreCase );

    // Returns whether the search is active (i.e. valid and non empty regexp)
    bool isActive() const
    {
        return active_;
    }

    // Return the text of the regex
    QString getPattern() const
    {
        return pattern_;
    }

    // THUONG_DEBUG
    void matchPreDefine( const QString& line, const QString &searchString, const Common::ChunkType type ,std::vector<QuickFindMatch>& matches ) const;

    // Returns whether the passed line match the quick find search.
    // If so, it populate the passed list with the list of matches
    // within this particular line.
    bool matchLine( const QString& line, std::vector<QuickFindMatch>& matches ) const;

    QuickFindMatcher getMatcher() const;

  signals:
    // Sent when the pattern is changed
    void patternUpdated();

  private:
    bool active_ = false;
    QRegularExpression regexp_;
    QString pattern_;
};

#endif
