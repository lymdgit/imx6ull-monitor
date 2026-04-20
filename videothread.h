#ifndef VIDEOTHREAD_H
#define VIDEOTHREAD_H

// ================================================================
//  工作模式选择（取消注释对应宏来切换模式）：
//
//  【本地显示模式（默认）】不推流，画面显示在 Qt 窗口，30fps
//  【推流模式】          取消注释 ENABLE_STREAM，推 RTSP 到
//                        MediaMTX，不显示本地画面，30fps
// ================================================================
 #define ENABLE_STREAM

// 统一目标帧率（本地显示 / 推流均使用此值）
#define TARGET_FPS          30
#define FRAME_INTERVAL_MS   (1000 / TARGET_FPS)   // 约 33 ms

#ifdef ENABLE_STREAM
// ---- 推流目标地址 ----
// 请将下方 IP 替换为运行 MediaMTX 的虚拟机实际 IP
#define STREAM_RTSP_URL  "rtsp://192.168.5.11:8554/live"
#endif
// ================================================================

#include <QThread>
#include <QByteArray>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>
#include <string.h>
#include <sys/mman.h>

class VideoThread : public QThread
{
    Q_OBJECT
public:
    explicit VideoThread(QObject *parent = nullptr);
    ~VideoThread();

    bool openCamera(int width, int height);
    void closeCamera();
    
    // 控制线程退出
    void stop();

protected:
    void run() override;

signals:
    // 将一帧完整的 MJPEG 数据发往主界面
    void newFrame(QByteArray frameData);

private:
    int video_fd;
    char *userbuff[4];
    int userbuff_length[4];
    bool m_running;
    int m_width;
    int m_height;
};

#endif // VIDEOTHREAD_H
