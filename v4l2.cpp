#include "v4l2.h"
#include "ui_v4l2.h"
#include <QDateTime>
#include <QRandomGenerator>
#include <QDir>
#include <stdio.h>
#include "videothread.h"  // 引入模式宏

v4l2::v4l2(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::v4l2)
{
    ui->setupUi(this);

    m_videoThread = new VideoThread(this);
    connect(m_videoThread, &VideoThread::newFrame, this, &v4l2::onNewFrame);

    // 自动打开摄像头
    if (m_videoThread->openCamera(video_width, video_height)) {
        printf("打开相机成功!\n");
        m_videoThread->start();
        start = 1;
        ui->pushButton_open->setText("关闭");

#ifdef ENABLE_STREAM
        // 推流模式：本地不显示画面，label 仅提示状态
        ui->label->setAlignment(Qt::AlignCenter);
        ui->label->setText(QString("推流中...\n目标：%1\n帧率：%2 fps")
                           .arg(STREAM_RTSP_URL)
                           .arg(TARGET_FPS));
#endif
    } else {
        printf("打开相机失败!\n");
    }

    // 创建 photo 目录
    QString photoDirPath = QDir::currentPath() + "/photo";
    QDir photoDir(photoDirPath);
    if (!photoDir.exists()) {
        if (photoDir.mkpath(photoDirPath))
            printf("创建 photo 文件夹成功\n");
        else
            printf("创建 photo 文件夹失败\n");
    }
}

v4l2::~v4l2()
{
    if (start == 1) {
        m_videoThread->stop();
        m_videoThread->wait();
        m_videoThread->closeCamera();
    }
    delete ui;
}

/* 显示一帧 */
void v4l2::onNewFrame(QByteArray frameData)
{
    QPixmap pix;
    if (pix.loadFromData((const uchar *)frameData.constData(), frameData.size())) {
        ui->label->setPixmap(pix);
    }
}

/* 打开 / 关闭相机 */
void v4l2::on_pushButton_open_clicked()
{
    if (start == 0) {
        if (m_videoThread->openCamera(video_width, video_height)) {
            printf("打开相机成功!\n");
            m_videoThread->start();
            start = 1;
            ui->pushButton_open->setText("关闭");
        }
    } else {
        m_videoThread->stop();
        m_videoThread->wait();
        m_videoThread->closeCamera();
        start = 0;
        ui->pushButton_open->setText("打开");
    }
}

/* 拍照 */
void v4l2::on_pushButton_take_clicked()
{
    QString randomNumbers;
    for (int i = 0; i < 10; i++)
        randomNumbers.append(QString::number(QRandomGenerator::global()->bounded(10)));

    QString str = "./photo/photo_" + randomNumbers + ".jpg";

    const QPixmap *pix = ui->label->pixmap();
    if (pix && !pix->isNull()) {
        pix->save(str, "JPG");
        printf("%s 保存成功\n", str.toStdString().c_str());
    } else {
        printf("无有效画面，拍照失败\n");
    }

    ui->label->clear();
}

/* 返回（关闭程序） */
void v4l2::on_pushButton_back_clicked()
{
    if (start == 1) {
        m_videoThread->stop();
        m_videoThread->wait();
        m_videoThread->closeCamera();
    }
    this->close();
}

/* 跳转到相册 */
void v4l2::on_pushButton_photos_clicked()
{
    if (start == 1) {
        m_videoThread->stop();
        m_videoThread->wait();
        m_videoThread->closeCamera();
        start = 0;
    }
    this->close();
    showphoto *s = new showphoto();
    s->show();
}
