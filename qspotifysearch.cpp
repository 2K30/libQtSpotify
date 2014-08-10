/****************************************************************************
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Yoann Lopes (yoann.lopes@nokia.com)
**
** This file is part of the MeeSpot project.
**
** Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions
** are met:
**
** Redistributions of source code must retain the above copyright notice,
** this list of conditions and the following disclaimer.
**
** Redistributions in binary form must reproduce the above copyright
** notice, this list of conditions and the following disclaimer in the
** documentation and/or other materials provided with the distribution.
**
** Neither the name of Nokia Corporation and its Subsidiary(-ies) nor the names of its
** contributors may be used to endorse or promote products derived from
** this software without specific prior written permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
** FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
** TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
** PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
** LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
** NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
** SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
****************************************************************************/


#include "qspotifysearch.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QEvent>
#include <QtCore/QMutexLocker>

#include <libspotify/api.h>

#include "qspotifyalbum.h"
#include "qspotifyartist.h"
#include "qspotifyplaylist.h"
#include "qspotifyplaylistsearchentry.h"
#include "qspotifysession.h"
#include "qspotifytrack.h"
#include "qspotifytracklist.h"
#include "qspotifyuser.h"

static QHash<sp_search *, QSpotifySearch *> g_searchObjects;
static QMutex g_mutex;

static void callback_search_complete(sp_search *result, void *)
{
    QMutexLocker lock(&g_mutex);
    QSpotifySearch *s = g_searchObjects.value(result);
    if (s)
        QCoreApplication::postEvent(s, new QEvent(QEvent::User));
}

QSpotifySearch::QSpotifySearch(QObject *parent)
    : QObject(parent)
    , m_sp_search(nullptr)
    , m_busy(false)
    , m_tracksLimit(100)
    , m_albumsLimit(50)
    , m_artistsLimit(50)
    , m_playlistsLimit(50)
{
    m_trackResults = new QSpotifyTrackList(this);
}

QSpotifySearch::~QSpotifySearch()
{
    qDeleteAll(m_albumResults);
    m_albumResults.clear();
    qDeleteAll(m_artistResults);
    m_artistResults.clear();
    qDeleteAll(m_playlistResults);
    m_playlistResults.clear();
    clearSearch();
}

void QSpotifySearch::setQuery(const QString &q)
{
    if (q == m_query)
        return;

    m_query = q;
    emit queryChanged();
}

void QSpotifySearch::search()
{
    clearSearch();

    m_busy = true;
    emit busyChanged();

    if (!m_query.isEmpty()) {
        QMutexLocker lock(&g_mutex);
        m_sp_search = sp_search_create(QSpotifySession::instance()->m_sp_session, m_query.toUtf8().constData(), 0, m_tracksLimit, 0, m_albumsLimit, 0, m_artistsLimit, 0, m_playlistsLimit, SP_SEARCH_SUGGEST, callback_search_complete, nullptr);
        g_searchObjects.insert(m_sp_search, this);
    } else {
        populateResults();
    }
}

void QSpotifySearch::clearSearch()
{
    QMutexLocker lock(&g_mutex);
    if (m_sp_search)
        sp_search_release(m_sp_search);
    g_searchObjects.remove(m_sp_search);
    m_sp_search = nullptr;
}

bool QSpotifySearch::event(QEvent *e)
{
    if (e->type() == QEvent::User) {
        populateResults();
        e->accept();
        return true;
    }
    return QObject::event(e);
}

void QSpotifySearch::populateAlbums()
{
    // Delete previous results
    qDeleteAll(m_albumResults);
    m_albumResults.clear();

    // Populate albums
    int c = sp_search_num_albums(m_sp_search);
    for (int i = 0; i < c; ++i) {
        sp_album *a = sp_search_album(m_sp_search, i);
        if (!sp_album_is_available(a))
            continue;
        QSpotifyAlbum *album = new QSpotifyAlbum(a);
        m_albumResults.append((QObject *)album);
    }
}

void QSpotifySearch::populateArtists()
{
    // Delete previous results
    qDeleteAll(m_artistResults);
    m_artistResults.clear();

    // Populate artists
    int c = sp_search_num_artists(m_sp_search);
    for (int i = 0; i < c; ++i) {
        QSpotifyArtist *artist = new QSpotifyArtist(sp_search_artist(m_sp_search, i));
        m_artistResults.append((QObject *)artist);
    }
}

void QSpotifySearch::populatePlaylists()
{
    // Delete previous results
    qDeleteAll(m_playlistResults);
    m_playlistResults.clear();

    // Populate playlists
    int c = sp_search_num_playlists(m_sp_search);
    for (int i = 0; i < c; ++i) {
        QSpotifyPlaylistSearchEntry *playlist = new QSpotifyPlaylistSearchEntry(sp_search_playlist_name(m_sp_search, i), sp_search_playlist(m_sp_search, i));
        m_playlistResults.append((QObject *)playlist);
    }
}

void QSpotifySearch::populateTracks()
{
    // Delete previous results
    if (m_trackResults) {
        m_trackResults->clear();
    }

    // Populate tracks
    int c = sp_search_num_tracks(m_sp_search);
    for (int i = 0; i < c; ++i) {
        std::shared_ptr<QSpotifyTrack> track(new QSpotifyTrack(sp_search_track(m_sp_search, i), m_trackResults), [] (QSpotifyTrack *track) {track->deleteLater();});
        track->metadataUpdated();
        m_trackResults->appendRow(track);
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksAdded(QVector<sp_track*>)), track.get(), SLOT(onStarredListTracksAdded(QVector<sp_track*>)));
        connect(QSpotifySession::instance()->user()->starredList(), SIGNAL(tracksRemoved(QVector<sp_track*>)), track.get(), SLOT(onStarredListTracksRemoved(QVector<sp_track*>)));
    }
}

void QSpotifySearch::populateResults()
{
    if (m_sp_search) {
        if (sp_search_error(m_sp_search) != SP_ERROR_OK)
            return;

        populateAlbums();
        populateArtists();
        populatePlaylists();
        populateTracks();

        m_didYouMean = QString::fromUtf8(sp_search_did_you_mean(m_sp_search));
    }

    m_busy = false;
    emit busyChanged();

    emit resultsChanged();
}
