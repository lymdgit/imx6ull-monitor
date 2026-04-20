#ifndef V4L2_H
#define V4L2_H

#include <QMainWindow>
#include <QTimer>
#include "videothread.h"
#include "showphoto.h"

#define video_width  1024
#define video_height 600

QT_BEGIN_NAMESPACE
namespace Ui { class v4l2; }
QT_END_NAMESPACE

class v4l2 : public QMainWindow
{
    Q_OBJECT

public:
    v4l2(QWidget *parent = nullptr);
    ~v4l2();

private slots:
    void on_pushButton_open_clicked();
    void on_pushButton_take_clicked();
    void on_pushButton_photos_clicked();
    void on_pushButton_back_clicked();

public slots:
    void onNewFrame(QByteArray frameData);

private:
    Ui::v4l2 *ui;

    int start = 0;
    VideoThread *m_videoThread;
};

#endif // V4L2_H
