/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#include "icon_builder.h"
icon_builder::icon_builder(const QIcon *base_icon, QString preset_key, int partitions, QWidget *parent) :
    QThread(parent), preset_key(preset_key), base_icon(*base_icon), partitions(partitions)
{
    connect(this, &icon_builder::finished, this, &icon_builder::deleteLater);
}

void icon_builder::run(){
    this->build_icons();
    emit this->set_complete();
    this->exit();
}

void icon_builder::build_icons() const {
    QList<QSize> sizes = this->base_icon.availableSizes(); //get the available sizes for the base icon
    QPixmap base_map = this->base_icon.pixmap(sizes[0]);
    QImage base = base_map.toImage();
    qDebug("%s", (std::stringstream() << "Base Image Dimensions: " << base.width() << "x" << base.height() << ", with depth " << base.depth() << ".").str().c_str());
    if(sizes[0] != base.size())
        qWarning("%s", (std::stringstream() << "Size of icon image [" << sizes[0].width() << ", " << sizes[0].height() << " does not match the derived image.").str().c_str());
    for(int p = 1; p <= this->partitions; p++){
        QImage img(base.size(), base.format());
        img.fill(Qt::transparent);
        qDebug("%s", (std::stringstream() << "Image " << p << " Dimensions: " << img.width() << "x" << img.height() << ", with depth " << img.depth() << ".").str().c_str());
        for(int i = 0; i < img.width(); i++){
            int j = 1;
            while(j <= base.height()){
                QColor base_color = base.pixelColor(i,base.height() - j);
                int h,s,v, a;
                base_color.getHsv(&h, &s, &v, &a);
                if (j <= ((base.height() / this->partitions) * p)){
                    if (base_color == Qt::transparent || base_color == Qt::color0)
                        img.setPixelColor(i,base.height() - j, base_color);
                    else{
                        v = (v >= 127) ? 255 : v + 128;
                        base_color.setHsv(h,s,v,a);
                        img.setPixelColor(i,base.height() - j, base_color);
                    }
                }
                else{
                    img.setPixelColor(i,base.height() - j, base.pixelColor(i,img.height() - j));
                }
                j++;
            }
        }
        emit this->icon_built(new const QIcon(QPixmap::fromImage(img)), this->preset_key);
    }
}
