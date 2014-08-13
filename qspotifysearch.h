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


#ifndef QSPOTIFYSEARCH_H
#define QSPOTIFYSEARCH_H

#include <QtCore/QObject>

class QSpotifyTrackList;
struct sp_search;

class QSpotifySearch : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QString query READ query WRITE setQuery NOTIFY queryChanged)
    Q_PROPERTY(QList<QObject *> albums READ albums NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> artists READ artists NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> playlists READ playlists NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> albumsPreview READ albumsPreview NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> artistsPreview READ artistsPreview NOTIFY resultsChanged)
    Q_PROPERTY(QList<QObject *> playlistsPreview READ playlistsPreview NOTIFY resultsChanged)
    Q_PROPERTY(QString didYouMean READ didYouMean NOTIFY resultsChanged)
    Q_PROPERTY(bool busy READ busy NOTIFY busyChanged)
public:
    QSpotifySearch(QObject *parent = nullptr);
    ~QSpotifySearch();

    QString query() const { return m_query; }
    void setQuery(const QString &q);

    QList<QObject *> albums() const { return m_albumResults; }
    QList<QObject *> artists() const { return m_artistResults; }
    QList<QObject *> playlists() const { return m_playlistResults; }

    QList<QObject *> albumsPreview() const { return m_albumResultsPreview; }
    QList<QObject *> artistsPreview() const { return m_artistResultsPreview; }
    QList<QObject *> playlistsPreview() const { return m_playlistResultsPreview; }

    QString didYouMean() const { return m_didYouMean; }

    void setTracksLimit(int l) { m_tracksLimit = l; }
    void setAlbumsLimit(int l) { m_albumsLimit = l; }
    void setArtistsLimit(int l) { m_artistsLimit = l; }
    void setPlaylistLimit(int l) { m_artistsLimit = l; }

    Q_INVOKABLE QSpotifyTrackList *trackResults() const { return m_trackResults; }
    Q_INVOKABLE QSpotifyTrackList *trackResultsPreview() const { return m_trackResultsPreview; }

    bool busy() const { return m_busy; }

    Q_INVOKABLE void search(bool preview = false);
    Q_INVOKABLE void searchAlbums();
    Q_INVOKABLE void searchArtists();
    Q_INVOKABLE void searchPlaylists();
    Q_INVOKABLE void searchTracks();

    bool event(QEvent *);

Q_SIGNALS:
    void queryChanged();
    void resultsChanged();
    void busyChanged();

private:
    void clearSearch();
    void clearList(QList<QObject *> list);

    void populateAlbums();
    void populateArtists();
    void populatePlaylists();
    void populateTracks();

    void setDidYouMean();
    void populateResults();

    void setBusy(bool busy);

    sp_search *m_sp_search;

    QString m_query;
    QSpotifyTrackList *m_trackResults;
    QList<QObject *> m_albumResults;
    QList<QObject *> m_artistResults;
    QList<QObject *> m_playlistResults;
    QString m_didYouMean;
    bool m_busy;

    // Preview
    QSpotifyTrackList *m_trackResultsPreview;
    QList<QObject *> m_albumResultsPreview;
    QList<QObject *> m_artistResultsPreview;
    QList<QObject *> m_playlistResultsPreview;

    int m_tracksLimit;
    int m_albumsLimit;
    int m_artistsLimit;
    int m_playlistsLimit;
    const int m_numPreviewItems;
};

#endif // QSPOTIFYSEARCH_H
