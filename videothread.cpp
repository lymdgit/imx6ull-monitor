#include "videothread.h"
#include <QElapsedTimer>
#include <stdio.h>

#ifdef ENABLE_STREAM
#include <cstdio>   // popen / pclose / fwrite / fflush
#include <cstring>  // snprintf
#endif

VideoThread::VideoThread(QObject *parent)
    : QThread(parent)
    , video_fd(-1)
    , m_running(false)
    , m_width(1024)
    , m_height(600)
{
    memset(userbuff, 0, sizeof(userbuff));
    memset(userbuff_length, 0, sizeof(userbuff_length));
}

VideoThread::~VideoThread()
{
    stop();
    wait();
    closeCamera();
}

bool VideoThread::openCamera(int width, int height)
{
    m_width = width;
    m_height = height;

    video_fd = open("/dev/video1", O_RDWR);
    if (video_fd < 0) {
        perror("打开摄像头设备失败");
        return false;
    }

    struct v4l2_capability capability;
    if (0 == ioctl(video_fd, VIDIOC_QUERYCAP, &capability)) {
        if ((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) == 0) {
            perror("该设备不支持视频采集");
            closeCamera();
            return false;
        }
    }

    struct v4l2_format format;
    memset(&format, 0, sizeof(format));
    format.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width       = m_width;
    format.fmt.pix.height      = m_height;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_MJPEG;
    if (0 > ioctl(video_fd, VIDIOC_S_FMT, &format)) {
        perror("设置摄像头格式失败");
        closeCamera();
        return false;
    }

    if (ioctl(video_fd, VIDIOC_G_FMT, &format) == 0)
        printf("实际分辨率: %d x %d\n", format.fmt.pix.width, format.fmt.pix.height);

    struct v4l2_requestbuffers reqbufs;
    memset(&reqbufs, 0, sizeof(reqbufs));
    reqbufs.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    reqbufs.count  = 4;
    reqbufs.memory = V4L2_MEMORY_MMAP;

    if (0 > ioctl(video_fd, VIDIOC_REQBUFS, &reqbufs)) {
        perror("申请内核缓冲区失败");
        closeCamera();
        return false;
    }

    for (int i = 0; i < (int)reqbufs.count; i++) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.index  = i;
        buf.memory = V4L2_MEMORY_MMAP;

        if (0 == ioctl(video_fd, VIDIOC_QUERYBUF, &buf)) {
            userbuff[i]        = (char *)mmap(NULL, buf.length,
                                               PROT_READ | PROT_WRITE,
                                               MAP_SHARED, video_fd, buf.m.offset);
            userbuff_length[i] = buf.length;
        }
        ioctl(video_fd, VIDIOC_QBUF, &buf);
    }

    int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    if (0 > ioctl(video_fd, VIDIOC_STREAMON, &type)) {
        perror("开启视频流失败");
        closeCamera();
        return false;
    }

    return true;
}

void VideoThread::closeCamera()
{
    if (video_fd >= 0) {
        int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        ioctl(video_fd, VIDIOC_STREAMOFF, &type);

        for (int i = 0; i < 4; i++) {
            if (userbuff[i]) {
                munmap(userbuff[i], userbuff_length[i]);
                userbuff[i] = nullptr;
            }
        }
        ::close(video_fd);
        video_fd = -1;
        printf("相机已关闭\n");
    }
}

void VideoThread::stop()
{
    m_running = false;
}

void VideoThread::run()
{
    if (video_fd < 0) return;

    m_running = true;

#ifdef ENABLE_STREAM
    // ---- 推流模式 ----
    // 通过 popen 启动 FFmpeg，将 MJPEG 帧写入其 stdin，
    // FFmpeg 负责封装并通过 RTSP 推送到 MediaMTX。
    char cmd[512];
    snprintf(cmd, sizeof(cmd),
             "ffmpeg -loglevel warning "
             "-f mjpeg -framerate %d -i pipe:0 "
             "-c:v copy "
             "-f rtsp -rtsp_transport tcp "
             "%s",
             TARGET_FPS, STREAM_RTSP_URL);

    printf("[推流] 启动 FFmpeg: %s\n", cmd);
    FILE *ffpipe = popen(cmd, "w");
    if (!ffpipe) {
        perror("[推流] 无法启动 FFmpeg 进程");
        return;
    }
    printf("[推流] FFmpeg 进程已启动，正在推流到 %s\n", STREAM_RTSP_URL);

    QElapsedTimer timer;
    timer.start();

    while (m_running) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (0 == ioctl(video_fd, VIDIOC_DQBUF, &buf)) {
            // 帧率门控：保持 TARGET_FPS，多余的帧直接丢弃
            if (buf.bytesused > 0 && timer.elapsed() >= FRAME_INTERVAL_MS) {
                size_t written = fwrite(userbuff[buf.index], 1, buf.bytesused, ffpipe);
                if (written != buf.bytesused)
                    fprintf(stderr, "[推流] 写入 FFmpeg 管道不完整: %zu/%u\n",
                            written, buf.bytesused);
                fflush(ffpipe);
                timer.restart();
            }

            if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf))
                perror("VIDIOC_QBUF 失败");
        } else {
            usleep(5000);
        }
    }

    pclose(ffpipe);
    printf("[推流] FFmpeg 进程已关闭\n");

#else
    // ---- 本地显示模式 ----
    QElapsedTimer timer;
    timer.start();

    while (m_running) {
        struct v4l2_buffer buf;
        memset(&buf, 0, sizeof(buf));
        buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory = V4L2_MEMORY_MMAP;

        if (0 == ioctl(video_fd, VIDIOC_DQBUF, &buf)) {
            // 帧率门控：保持 TARGET_FPS
            if (buf.bytesused > 0 && timer.elapsed() >= FRAME_INTERVAL_MS) {
                QByteArray frameData(userbuff[buf.index], buf.bytesused);
                emit newFrame(frameData);
                timer.restart();
            }

            if (0 > ioctl(video_fd, VIDIOC_QBUF, &buf))
                perror("VIDIOC_QBUF 失败");
        } else {
            usleep(5000);
        }
    }
#endif
}
