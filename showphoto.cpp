#include "showphoto.h"
#include "ui_showphoto.h"
#include <v4l2.h>

showphoto::showphoto(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::showphoto)
{
    printf("showphoto ui运行\n");
    ui->setupUi(this);

    dir.setPath("./photo/"); // 设置目录路径
    dir.setFilter(QDir::Files | QDir::NoDotAndDotDot); // 过滤条件，只获取文件
    dir.setNameFilters(QStringList() << "*.jpg"); // 文件名过滤，只获取jpg文件
    dir.setSorting(QDir::Time | QDir::Reversed);  // 根据修改时间进行排序，逆序排列
    fileList = dir.entryList(); // 获取文件列表

    //先显示一张图片
    if(fileList.length() > 0){
        QString filePath = dir.filePath(fileList[currentIndex]); // 获取文件路径
        QPixmap pixmap(filePath); // 创建图片对象
        ui->label->setPixmap(pixmap); // 在控件上显示图片
        currentIndex++; // 更新当前显示的文件索引
        if (currentIndex >= fileList.size()) {
            currentIndex = 0;
        }
    }

}

showphoto::~showphoto()
{
    delete ui;
}

/* 回到上一个界面 */
void showphoto::on_pushButton_back_clicked()
{
    this->close();
//    v4l2 v;
//    v.show();
    v4l2 *v = new v4l2();   //new会分配内存
    v->show();
}


/* 下一张图片 */
void showphoto::on_pushButton_next_clicked()
{
    if(fileList.length() > 0){
        QString filePath = dir.filePath(fileList[currentIndex]); // 获取文件路径
        QPixmap pixmap(filePath); // 创建图片对象
        ui->label->setPixmap(pixmap); // 在控件上显示图片
        currentIndex++; // 更新当前显示的文件索引
        if (currentIndex >= fileList.size()) {
            currentIndex = 0;
        }
    }
}

/* 上一张图片 */
void showphoto::on_pushButton_front_clicked()
{
    if(fileList.length() > 0){
        QString filePath = dir.filePath(fileList[currentIndex]); // 获取文件路径
        QPixmap pixmap(filePath); // 创建图片对象
        ui->label->setPixmap(pixmap); // 在控件上显示图片
        currentIndex--; // 更新当前显示的文件索引
        if (currentIndex < 0) {
            currentIndex = fileList.size()-1;
        }
    }
}
