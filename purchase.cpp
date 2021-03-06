﻿#include "purchase.h"
#include "ui_purchase.h"
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QBuffer>
#pragma execution_character_set("utf-8")

Purchase::Purchase(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Purchase)
{
    ui->setupUi(this);
    req = new QNetworkAccessManager(this);
    QObject::connect(req, SIGNAL(finished(QNetworkReply*)),
                 this, SLOT(firstFinished(QNetworkReply*)));
}

Purchase::~Purchase()
{
    delete ui;
}
void Purchase::init(int num, QString username){
    ui->lineEdit->setText(QString::number(num));
    ui->label_3->setText("￥"+QString::number(num*0.1));
    price = QString::number(num*0.1*100);
}

void Purchase::loadQRCode(){
    QUrl url("https://open.youzan.com/oauth/token");
        QByteArray append("client_id=c2fcaaaa056cf199f8&client_secret=ef4b7c5bb70112f7f456b58d503c3477&grant_type=silent&kdt_id=40044043");
        req->post(QNetworkRequest(url), append);
}

void Purchase::firstFinished(QNetworkReply *reply){
    if(first){
        if (reply->error() == QNetworkReply::NoError){
                 QByteArray bytes = reply->readAll();
                 QString string = QString::fromUtf8(bytes);
                 QJsonParseError jsonError;
                 QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);  // 转化为 JSON 文档
                 if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
                     if (doucment.isObject()) { // JSON 文档为对象
                         QJsonObject object = doucment.object();  // 转化为对象
                         if (object.contains("access_token")) {  // 包含指定的 key
                             token = object.value("access_token").toString();
                             first=0;
                             dropOrder();
                          }
                     }
                 }
             }
        reply->deleteLater();
    }else{
         if (reply->error() == QNetworkReply::NoError){
                QByteArray bytes = reply->readAll();
                QJsonParseError jsonError;
                QJsonDocument doucment = QJsonDocument::fromJson(bytes, &jsonError);  // 转化为 JSON 文档
                if (!doucment.isNull() && (jsonError.error == QJsonParseError::NoError)) {  // 解析未发生错误
                    if (doucment.isObject()) {
                        QJsonObject object = doucment.object();
                        if (object.contains("response")) {
                            QString code = object.value("response").toObject().value("qr_code").toString();
                            QString id = object.value("response").toObject().value("qr_id").toString();
                            first=0;
                            Base64_To_Image(code.replace("data:image/png;base64,","").toUtf8(),"qr.png");
                            qDebug()<<code;
                            ui->label_5->setPixmap(QPixmap("qr.png"));
                         }
                    }
                }
         }
    }
}

void Purchase::dropOrder(){
    QUrl url("https://open.youzan.com/api/oauthentry/youzan.pay.qrcode/3.0.0/create");
        QByteArray append(QString("qr_name=Telegraph积分充值&qr_price="+price+"&qr_type=QR_TYPE_DYNAMIC&access_token="+token).toUtf8());
        req->post(QNetworkRequest(url), append);
}

QPixmap Purchase::Base64_To_Image(QByteArray bytearray,QString SavePath)
{
    QByteArray Ret_bytearray;
    Ret_bytearray = QByteArray::fromBase64(bytearray);
    QBuffer buffer(&Ret_bytearray);
    buffer.open(QIODevice::WriteOnly);
    QPixmap imageresult;
    imageresult.loadFromData(Ret_bytearray);
    if(SavePath != "")
    {
        qDebug() <<"save" ;
        imageresult.save(SavePath);
    }
    return imageresult;
}
