#include "settingsdialog.h"
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent), ui(nullptr) {
    setWindowTitle("Настройки");
    auto *layout = new QVBoxLayout(this);
    auto *cbSound = new QCheckBox("Включить звук (если добавим)", this);
    cbSound->setChecked(true);
    auto *cbGrid = new QCheckBox("Показывать сетку", this);
    cbGrid->setChecked(true);
    layout->addWidget(cbSound);
    layout->addWidget(cbGrid);

    auto *btnBox = new QHBoxLayout();
    auto *ok = new QPushButton("OK", this);
    auto *cancel = new QPushButton("Отмена", this);
    btnBox->addWidget(ok);
    btnBox->addWidget(cancel);
    layout->addLayout(btnBox);

    connect(ok, &QPushButton::clicked, this, &SettingsDialog::accept);
    connect(cancel, &QPushButton::clicked, this, &SettingsDialog::reject);

    // Храним виджеты в свойствах окна, чтобы можно было прочитать позже
    setProperty("sound_cb", QVariant::fromValue(static_cast<QObject*>(cbSound)));
    setProperty("grid_cb", QVariant::fromValue(static_cast<QObject*>(cbGrid)));
}

SettingsDialog::~SettingsDialog() {
    // ui не используется
}

bool SettingsDialog::soundEnabled() const {
    auto obj = property("sound_cb").value<QObject*>();
    if (!obj) return true;
    auto cb = qobject_cast<QCheckBox*>(obj);
    return cb ? cb->isChecked() : true;
}

bool SettingsDialog::showGrid() const {
    auto obj = property("grid_cb").value<QObject*>();
    if (!obj) return true;
    auto cb = qobject_cast<QCheckBox*>(obj);
    return cb ? cb->isChecked() : true;
}
