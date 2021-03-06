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

#include "filewatcher.h"

#include "configuration.h"
#include "log.h"

#include <efsw/efsw.hpp>
#include <vector>

#include <QDateTime>
#include <QDir>
#include <QFileInfo>
#include <QMutex>
#include <QMutexLocker>
#include <QTimer>

namespace {

struct WatchedFile {
    std::string name;
    int64_t mTime;
    int64_t size;

    bool operator==( const std::string& filename ) const
    {
        return name == filename;
    }

    bool operator!=( const WatchedFile& other ) const
    {
        return name != other.name || mTime != other.mTime || size != other.size;
    }
};

struct WatchedDirecotry {
    efsw::WatchID watchId;

    // filenames are in utf8
    std::string name;
    std::vector<WatchedFile> files;
};

bool isOnlyForPolling( const WatchedDirecotry& wd )
{
    return wd.watchId < 0;
}

} // namespace

class EfswFileWatcher final : public efsw::FileWatchListener {
  public:
    explicit EfswFileWatcher( FileWatcher* parent )
        : parent_{ parent }
        , mutex_{ QMutex::Recursive }
    {
    }

    void enableWatch( bool enable )
    {
        QMutexLocker lock( &mutex_ );

        if ( nativeWatchEnabled_ == enable ) {
            return;
        }

        nativeWatchEnabled_ = enable;

        if ( enable ) {
            for ( auto& dir : watchedPaths_ ) {
                LOG( logINFO ) << "Will reenable watch for " << dir.name;
                dir.watchId = watcher_.addWatch( dir.name, this, false );
            }
        }
        else {
            for ( auto& dir : watchedPaths_ ) {
                LOG( logINFO ) << "Will disable watch for " << dir.name;
                watcher_.removeWatch( dir.watchId );
            }
        }
    }

    void addFile( const QString& fullFileName )
    {
        QMutexLocker lock( &mutex_ );

        LOG( logDEBUG ) << fullFileName.toStdString();

        const QFileInfo fileInfo = QFileInfo( fullFileName );

        auto watchedFile
            = WatchedFile{ fileInfo.fileName().toStdString(),
                           fileInfo.lastModified().toMSecsSinceEpoch(), fileInfo.size() };

        const auto directory = fileInfo.canonicalPath().toStdString();

        const auto wasEmpty = watchedPaths_.empty();

        auto watchedDirectory
            = std::find_if( watchedPaths_.begin(), watchedPaths_.end(),
                            [&directory]( const auto& wd ) { return wd.name == directory; } );

        const auto tryWatchDirectory = [this]( const std::string& path ) {
            auto watchId = nativeWatchEnabled_ ? watcher_.addWatch( path, this, false ) : -111;

            if ( watchId < 0 ) {
                LOG( logWARNING ) << "failed to add watch " << path << " error " << watchId;
            }

            return watchId;
        };

        if ( watchedDirectory == watchedPaths_.end() ) {
            watchedPaths_.push_back(
                { tryWatchDirectory( directory ), directory, { std::move( watchedFile ) } } );
        }
        else {

            if ( isOnlyForPolling( *watchedDirectory ) ) {
                watchedDirectory->watchId = tryWatchDirectory( directory );
            }

            if ( std::find( watchedDirectory->files.begin(), watchedDirectory->files.end(),
                            watchedFile.name )
                 != watchedDirectory->files.end() ) {
                LOG( logDEBUG ) << "already watching " << watchedFile.name << " in " << directory;
                return;
            }

            watchedDirectory->files.emplace_back( std::move( watchedFile ) );
        }

        if ( wasEmpty ) {
            watcher_.watch();
        }
    }

    void removeFile( const QString& fullFileName )
    {
        QMutexLocker lock( &mutex_ );

        LOG( logDEBUG ) << fullFileName.toStdString();

        const QFileInfo fileInfo = QFileInfo( fullFileName );

        const auto filename = fileInfo.fileName().toStdString();
        const auto directory = fileInfo.absolutePath().toStdString();

        auto watchedDirectory
            = std::find_if( watchedPaths_.begin(), watchedPaths_.end(),
                            [&directory]( const auto& wd ) { return wd.name == directory; } );

        if ( watchedDirectory != watchedPaths_.end() ) {

            auto watchedFile = std::find( watchedDirectory->files.begin(),
                                          watchedDirectory->files.end(), filename );

            if ( watchedFile != watchedDirectory->files.end() ) {
                watchedDirectory->files.erase( watchedFile );
            }

            if ( watchedDirectory->files.empty() ) {

                if ( !isOnlyForPolling( *watchedDirectory ) ) {
                    watcher_.removeWatch( watchedDirectory->watchId );
                }

                watchedPaths_.erase( watchedDirectory );
            }
        }
        else {
            LOG( logWARNING ) << "The file is not watched";
        }

        for ( const auto& d : watcher_.directories() ) {
            LOG( logINFO ) << "Directories still watched: " << d;
        }
    }

    void checkWatches()
    {
        const auto collectChangedFiles = [this]() {
            QMutexLocker lock( &mutex_ );

            std::vector<QString> changedFiles;

            for ( auto& dir : watchedPaths_ ) {
                for ( auto& file : dir.files ) {
                    const auto path
                        = QDir::cleanPath( QString::fromStdString( dir.name ) + QDir::separator()
                                           + QString::fromStdString( file.name ) );

                    const auto fileInfo = QFileInfo{ path };

                    auto watchedFile = WatchedFile{ fileInfo.fileName().toStdString(),
                                                    fileInfo.lastModified().toMSecsSinceEpoch(),
                                                    fileInfo.size() };

                    if ( file != watchedFile ) {
                        changedFiles.push_back( path );
                        LOG( logINFO ) << "will notify for " << path;
                    }

                    file = std::move( watchedFile );
                }
            }

            return changedFiles;
        };

        for ( const auto& changedFile : collectChangedFiles() ) {
            QMetaObject::invokeMethod( parent_, "fileChangedOnDisk", Qt::QueuedConnection,
                                       Q_ARG( QString, changedFile ) );
        }
    }

    void handleFileAction( efsw::WatchID watchid, const std::string& dir,
                           const std::string& filename, efsw::Action action,
                           std::string oldFilename ) override
    {
        Q_UNUSED( watchid );
        Q_UNUSED( action );

        LOG( logDEBUG ) << "Notification from esfw for " << dir;

        // post to other thread to avoid deadlock between internal esfw lock and our mutex_
        auto signalSource = new QObject;
        QObject::connect( signalSource, &QObject::destroyed, parent_,
                          [=]( QObject* ) { notifyOnFileAction( dir, filename, oldFilename ); },
                          Qt::QueuedConnection );
        signalSource->moveToThread( parent_->thread() );
        signalSource->deleteLater();
    }

    void notifyOnFileAction( const std::string& dir, const std::string& filename,
                             const std::string& oldFilename )
    {
        auto qtDir = QString::fromStdString( dir );
        if ( qtDir.endsWith( QDir::separator() ) ) {
            qtDir.chop( 1 );
        }

        const auto directory = qtDir.toStdString();

        LOG( logDEBUG ) << "fileChangedOnDisk " << directory << " " << filename << ", old name "
                        << oldFilename;

        const auto fullChangedFilename = findChangedFilename( directory, filename, oldFilename );

        if ( !fullChangedFilename.isEmpty() ) {
            QMetaObject::invokeMethod( parent_, "fileChangedOnDisk", Qt::QueuedConnection,
                                       Q_ARG( QString, fullChangedFilename ) );
        }
    }

    QString findChangedFilename( const std::string& directory, const std::string& filename,
                                 const std::string& oldFilename )
    {
        QMutexLocker lock( &mutex_ );

        auto watchedDirectory
            = std::find_if( watchedPaths_.begin(), watchedPaths_.end(),
                            [&directory]( const auto& wd ) { return wd.name == directory; } );

        if ( watchedDirectory != watchedPaths_.end() ) {
            std::string changedFilename;

            const auto isFileWatched
                = std::any_of( watchedDirectory->files.begin(), watchedDirectory->files.end(),
                               [&filename, &oldFilename, &changedFilename]( const auto& f ) {
                                   if ( f.name == filename || f.name == oldFilename ) {
                                       changedFilename = f.name;
                                       return true;
                                   }

                                   return false;
                               } );

            if ( isFileWatched ) {
                LOG( logDEBUG ) << "fileChangedOnDisk - will notify for " << filename
                                << ", old name " << oldFilename;

                return QDir::cleanPath( QString::fromStdString( directory ) + QDir::separator()
                                        + QString::fromStdString( changedFilename ) );
            }
            else {
                LOG( logDEBUG ) << "fileChangedOnDisk - call but no file monitored";
            }
        }
        else {
            LOG( logDEBUG ) << "fileChangedOnDisk - call but no dir monitored";
        }

        return QString{};
    }

  private:
    efsw::FileWatcher watcher_;
    std::vector<WatchedDirecotry> watchedPaths_;
    FileWatcher* parent_;

    bool nativeWatchEnabled_ = true;

    QMutex mutex_;
};

void EfswFileWatcherDeleter::operator()( EfswFileWatcher* watcher ) const
{
    delete watcher;
}

FileWatcher::FileWatcher()
    : checkTimer_{ new QTimer( this ) }
    , notificationTimer_{ new QTimer( this ) }
    , efswWatcher_{ new EfswFileWatcher( this ) }
{
    connect( checkTimer_, &QTimer::timeout, this, &FileWatcher::checkWatches );
}

FileWatcher::~FileWatcher() {}

FileWatcher& FileWatcher::getFileWatcher()
{
    static auto* const instance = new FileWatcher;
    return *instance;
}

void FileWatcher::addFile( const QString& fileName )
{
    efswWatcher_->addFile( fileName );

    updateConfiguration();
}

void FileWatcher::removeFile( const QString& fileName )
{
    efswWatcher_->removeFile( fileName );

    updateConfiguration();
}

void FileWatcher::fileChangedOnDisk( const QString& fileName )
{
    if ( std::find( changes_.begin(), changes_.end(), fileName ) == changes_.end() ) {
        changes_.push_back( fileName );
    }

    if ( !notificationTimer_->isActive() ) {
        notificationTimer_->singleShot( 500, this, &FileWatcher::notifyFileChangedOnDisk );
    }
}

void FileWatcher::notifyFileChangedOnDisk()
{
    for ( const auto& fileName : changes_ ) {
        emit fileChanged( fileName );
    }

    changes_.clear();
}

void FileWatcher::updateConfiguration()
{
    const auto& config = Configuration::get();

    if ( config.pollingEnabled() ) {
        LOG( logINFO ) << "Polling files enabled";
        checkTimer_->start( config.pollIntervalMs() );
    }
    else {
        LOG( logINFO ) << "Polling files disabled";
        checkTimer_->stop();
    }

    efswWatcher_->enableWatch( config.nativeFileWatchEnabled() );
}

void FileWatcher::checkWatches()
{
    efswWatcher_->checkWatches();
}
