#ifndef QSPOTIFYAUDIOTHREADWORKER_H
#define QSPOTIFYAUDIOTHREADWORKER_H

#include <QtCore/QObject>
#include <QtCore/QBuffer>
#include <QtCore/QMutex>
#include <QtCore/QWaitCondition>
#include <QtCore/QHash>
#include <QtGui/QImage>
#include <libspotify/api.h>

#define BUFFER_SIZE 409600
#define AUDIOSTREAM_UPDATE_INTERVAL 20

extern QBuffer g_buffer;
extern QMutex g_mutex;
extern int g_readPos;
extern int g_writePos;

extern QMutex g_imageRequestMutex;
extern QHash<QString, QWaitCondition *> g_imageRequestConditions;
extern QHash<QString, QImage> g_imageRequestImages;
extern QHash<sp_image *, QString> g_imageRequestObject;

class QAudioOutput;
class QIODevice;

class QSpotifyAudioThreadWorker : public QObject
{
public:
    QSpotifyAudioThreadWorker(QObject *parent = nullptr);

    bool event(QEvent *);

private:
    void startStreaming(int channels, int sampleRate);
    void updateAudioBuffer();

    QAudioOutput *m_audioOutput;
    QIODevice *m_iodevice;
    int m_audioTimerID;
    int m_timeCounter;
    bool m_endOfTrack;
    int m_previousElapsedTime;
};

#endif // QSPOTIFYAUDIOTHREADWORKER_H
