//
//  RunningScriptsWidget.cpp
//  interface
//
//  Created by Mohammed Nafees on 03/28/2014.
//  Copyright (c) 2014 High Fidelity, Inc. All rights reserved.

#include "ui_runningScriptsWidget.h"
#include "RunningScriptsWidget.h"

#include <QKeyEvent>
#include <QFileInfo>
#include <QScrollArea>
#include <QPainter>
#include <QTableWidgetItem>

#include "Application.h"

RunningScriptsWidget::RunningScriptsWidget(QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::RunningScriptsWidget),
    _mousePressed(false),
    _mousePosition(QPoint())
{
    ui->setupUi(this);

    setWindowFlags(Qt::SubWindow | Qt::FramelessWindowHint);

    _runningScriptsTable = new ScriptsTableWidget(ui->runningScriptsTableWidget);
    _runningScriptsTable->setColumnCount(2);
    _runningScriptsTable->setColumnWidth(0, 245);
    _runningScriptsTable->setColumnWidth(1, 22);
    connect(_runningScriptsTable, &QTableWidget::cellClicked, this, &RunningScriptsWidget::stopScript);

    _recentlyLoadedScriptsTable = new ScriptsTableWidget(ui->recentlyLoadedScriptsTableWidget);
    _recentlyLoadedScriptsTable->setColumnCount(1);
    _recentlyLoadedScriptsTable->setColumnWidth(0, 265);
    connect(_recentlyLoadedScriptsTable, &QTableWidget::cellClicked,
            this, &RunningScriptsWidget::loadScript);

    connect(ui->hideWidgetButton, &QPushButton::clicked,
            Application::getInstance(), &Application::toggleRunningScriptsWidget);
    connect(ui->reloadAllButton, &QPushButton::clicked,
            Application::getInstance(), &Application::reloadAllScripts);
    connect(ui->stopAllButton, &QPushButton::clicked,
            this, &RunningScriptsWidget::allScriptsStopped);
}

RunningScriptsWidget::~RunningScriptsWidget()
{
    delete ui;
}

void RunningScriptsWidget::setBoundary(const QRect &rect)
{
    _boundary = rect;
}

void RunningScriptsWidget::setRunningScripts(const QStringList& list)
{
    _runningScriptsTable->setRowCount(list.size());

    ui->noRunningScriptsLabel->setVisible(list.isEmpty());
    ui->currentlyRunningLabel->setVisible(!list.isEmpty());
    ui->runningScriptsTableWidget->setVisible(!list.isEmpty());
    ui->reloadAllButton->setVisible(!list.isEmpty());
    ui->stopAllButton->setVisible(!list.isEmpty());

    for (int i = 0; i < list.size(); ++i) {
        QTableWidgetItem *scriptName = new QTableWidgetItem;
        scriptName->setText(QFileInfo(list.at(i)).fileName());
        scriptName->setToolTip(list.at(i));
        scriptName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        QTableWidgetItem *closeIcon = new QTableWidgetItem;
        closeIcon->setIcon(QIcon(QPixmap(":/images/kill-script.svg").scaledToHeight(12)));

        _runningScriptsTable->setItem(i, 0, scriptName);
        _runningScriptsTable->setItem(i, 1, closeIcon);
    }

    int y = ui->runningScriptsTableWidget->y() + 12;
    for (int i = 0; i < _runningScriptsTable->rowCount(); ++i) {
        y += _runningScriptsTable->rowHeight(i);
    }

    ui->runningScriptsTableWidget->resize(ui->runningScriptsTableWidget->width(), y - 12);
    _runningScriptsTable->resize(_runningScriptsTable->width(), y - 12);
    ui->reloadAllButton->move(ui->reloadAllButton->x(), y);
    ui->stopAllButton->move(ui->stopAllButton->x(), y);
    ui->recentlyLoadedLabel->move(ui->recentlyLoadedLabel->x(),
                                  ui->stopAllButton->y() + ui->stopAllButton->height() + 61);
    ui->recentlyLoadedScriptsTableWidget->move(ui->recentlyLoadedScriptsTableWidget->x(),
                                               ui->recentlyLoadedLabel->y() + 19);


    createRecentlyLoadedScriptsTable();
}

void RunningScriptsWidget::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        _mousePressed = true;
        _mousePosition = e->pos();
    } else {
        _mousePressed = false;
        _mousePosition = QPoint();
    }
    QWidget::mousePressEvent(e);
}

void RunningScriptsWidget::mouseMoveEvent(QMouseEvent *e)
{
    if (_mousePressed) {
        QPoint newPosition = mapToParent(e->pos() - _mousePosition);
        if (newPosition.x() >= _boundary.x() &&
            newPosition.x() <= (_boundary.width() - width())) {
            move(newPosition.x(), _boundary.y());
        }
    }
    QWidget::mouseMoveEvent(e);
}

void RunningScriptsWidget::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        _mousePressed = false;
        _mousePosition = QPoint();
    }
    QWidget::mouseReleaseEvent(e);
}

void RunningScriptsWidget::keyPressEvent(QKeyEvent *e)
{
    switch(e->key()) {
    case Qt::Key_Escape:
        Application::getInstance()->toggleRunningScriptsWidget();
        break;

    case Qt::Key_1:
        if (_recentlyLoadedScripts.size() > 0) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(0));
        }
        break;

    case Qt::Key_2:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 2) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(1));
        }
        break;

    case Qt::Key_3:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 3) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(2));
        }
        break;

    case Qt::Key_4:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 4) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(3));
        }
        break;
    case Qt::Key_5:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 5) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(4));
        }
        break;

    case Qt::Key_6:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 6) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(5));
        }
        break;

    case Qt::Key_7:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 7) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(6));
        }
        break;
    case Qt::Key_8:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 8) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(7));
        }
        break;

    case Qt::Key_9:
        if (_recentlyLoadedScripts.size() > 0 && _recentlyLoadedScripts.size() >= 9) {
            Application::getInstance()->loadScript(_recentlyLoadedScripts.at(8));
        }
        break;

    default:
        break;
    }
    QWidget::keyPressEvent(e);
}

void RunningScriptsWidget::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setPen(QColor::fromRgb(225, 225, 225)); // #e1e1e1

    if (ui->currentlyRunningLabel->isVisible()) {
        // line below the 'Currently Running' label
        painter.drawLine(36, ui->currentlyRunningLabel->y() + ui->currentlyRunningLabel->height(),
                         300, ui->currentlyRunningLabel->y() + ui->currentlyRunningLabel->height());
    }

    if (ui->recentlyLoadedLabel->isVisible()) {
        // line below the 'Recently loaded' label
        painter.drawLine(36, ui->recentlyLoadedLabel->y() + ui->recentlyLoadedLabel->height(),
                         300, ui->recentlyLoadedLabel->y() + ui->recentlyLoadedLabel->height());
    }

    painter.end();
}

void RunningScriptsWidget::stopScript(int row, int column)
{
    if (column == 1) { // make sure the user has clicked on the close icon
        _lastStoppedScript = _runningScriptsTable->item(row, 0)->toolTip();
        emit stopScriptName(_runningScriptsTable->item(row, 0)->toolTip());
    }
}

void RunningScriptsWidget::loadScript(int row, int column)
{
    Application::getInstance()->loadScript(_recentlyLoadedScriptsTable->item(row, column)->toolTip());
}

void RunningScriptsWidget::allScriptsStopped()
{
    QStringList list = Application::getInstance()->getRunningScripts();
    for (int i = 0; i < list.size(); ++i) {
        _recentlyLoadedScripts.prepend(list.at(i));
    }

    Application::getInstance()->stopAllScripts();
}

void RunningScriptsWidget::createRecentlyLoadedScriptsTable()
{
    if (!_recentlyLoadedScripts.contains(_lastStoppedScript) && !_lastStoppedScript.isEmpty()) {
        _recentlyLoadedScripts.prepend(_lastStoppedScript);
        _lastStoppedScript = "";
    }

    for (int i = 0; i < _recentlyLoadedScripts.size(); ++i) {
        if (Application::getInstance()->getRunningScripts().contains(_recentlyLoadedScripts.at(i))) {
            _recentlyLoadedScripts.removeOne(_recentlyLoadedScripts.at(i));
        }
    }

    ui->recentlyLoadedLabel->setVisible(!_recentlyLoadedScripts.isEmpty());
    ui->recentlyLoadedScriptsTableWidget->setVisible(!_recentlyLoadedScripts.isEmpty());
    ui->recentlyLoadedInstruction->setVisible(!_recentlyLoadedScripts.isEmpty());

    int limit = _recentlyLoadedScripts.size() > 9 ? 9 : _recentlyLoadedScripts.size();
    _recentlyLoadedScriptsTable->setRowCount(limit);
    for (int i = 0; i < limit; ++i) {
        QTableWidgetItem *scriptName = new QTableWidgetItem;
        scriptName->setText(QString::number(i+1) + ". " +QFileInfo(_recentlyLoadedScripts.at(i)).fileName());
        scriptName->setToolTip(_recentlyLoadedScripts.at(i));
        scriptName->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);

        _recentlyLoadedScriptsTable->setItem(i, 0, scriptName);
    }

    int y = ui->recentlyLoadedScriptsTableWidget->y() + 15;
    for (int i = 0; i < _recentlyLoadedScriptsTable->rowCount(); ++i) {
        y += _recentlyLoadedScriptsTable->rowHeight(i);
    }

    ui->recentlyLoadedInstruction->setGeometry(36, y,
                                               ui->recentlyLoadedInstruction->width(),
                                               ui->recentlyLoadedInstruction->height());

    repaint();
}
