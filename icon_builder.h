/*
 * SPDX-FileCopyrightText: © 2024 - Samuel Fincher <Smfincher@yahoo.com>
 * SPDX-License-Identifier:  AGPL-3.0-only
 */
#ifndef ICON_BUILDER_H
#define ICON_BUILDER_H
#include "widgets.h"

/* icon_builder: Constructs icon variants for when x/<partition> of a segment's timer has elapsed.
 *  - This is done in a new thread to avoid locking up the timer while the icons are being built.
 *  - On completing an icon, it is sent to the main preset_icon_manager instance.
 *  - On completing all icons, the set_complete signal is sent, and the class self-destructs.
 */
class icon_builder : public QThread
{
    Q_OBJECT
public:
    icon_builder(const QIcon *base_icon, QString preset_key, int partitions = 4, QWidget *parent = nullptr);
    void run() override;
signals:
    void icon_built(const QIcon*, QString key) const;
    void set_complete();
private:
    void build_icons() const;
    const QString preset_key;
    const QIcon base_icon;
    const double partitions;
};

#endif // ICON_BUILDER_H
