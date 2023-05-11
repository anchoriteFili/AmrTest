#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTextCodec>
#include <opencore-amrnb/interf_dec.h>
#include <stdio.h>
#include "opencore/wav.h"

const int sizes[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    player = new QMediaPlayer();
}

MainWindow::~MainWindow()
{
    delete ui;

}

// 播放按钮点击
void MainWindow::on_play_clicked()
{

    QTextCodec *code = QTextCodec::codecForName("GB2312");
    std::string name = code->fromUnicode("E:/AmrTest/input.amr").data();

    // name需要处理一下
    FILE* in = fopen(name.c_str(), "rb");
    if (!in) {
        return;
    }
    char header[6];
    size_t n = fread(header, 1, 6, in);
    if (n != 6 || memcmp(header, "#!AMR\n", 6)) {
        fprintf(stderr, "Bad header\n");
        return;
    }

    // 本地文件路径
    QString fileName = "E:/AmrTest/output.wav";
    // 将wav放进了本地文件
    WavWriter wav(std::string(fileName.toLocal8Bit()).c_str(),8000,16,1);

    void* amr = Decoder_Interface_init();
    while (1) {
        uint8_t buffer[500];
        /* Read the mode byte */
        n = fread(buffer, 1, 1, in);
        if (n <= 0)
            break;
        /* Find the packet size */
        int size = sizes[(buffer[0] >> 3) & 0x0f];
        if (size <= 0)
            continue;
        n = fread(buffer + 1, 1, size, in);
        if (n != size)
            break;
        /* Decode the packet */
        int16_t outbuffer[160];
        Decoder_Interface_Decode(amr, buffer, outbuffer, 0);
        /* Convert to little endian and write to wav */
        uint8_t littleendian[320];
        uint8_t* ptr = littleendian;
        for (int i = 0; i < 160; i++) {
            *ptr++ = (outbuffer[i] >> 0) & 0xff;
            *ptr++ = (outbuffer[i] >> 8) & 0xff;
        }

        wav.writeData(littleendian, 320);
    }
    fclose(in);
    Decoder_Interface_exit(amr);

    player->setMedia(QUrl(fileName));
    player->play();


}

