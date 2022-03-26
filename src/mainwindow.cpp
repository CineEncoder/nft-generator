#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "message.h"
#include "dialog.h"
#include <QDebug>


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    generatedPathsList(QList<QList<NumPaths>>()),
    ui(new Ui::MainWindow),
    window_activated_flag(false),
    lock_ar_flag(true),
    folder_added_flag(false),
    current_image(0),
    aspectRatio(1.f)
{
    ui->setupUi(this);
    this->setFocusPolicy(Qt::StrongFocus);
    generator = new Generator(this);
    ptr_progress = new ProgressMessage(this);
    ptr_progress->hide();
    ui->buttonLockAR->setProperty("lock", true);

    this->installEventFilter(this);
    ui->graphicsView->installEventFilter(this);
    ui->graphicsView->verticalScrollBar()->installEventFilter(this);
    ui->graphicsView->horizontalScrollBar()->installEventFilter(this);
    ui->graphicsView->setMouseTracking(true);
    ui->graphicsView->setInteractive(true);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag);
    ui->graphicsView->setFocusPolicy(Qt::NoFocus);

    scene = new QGraphicsScene(this);
    ui->graphicsView->setScene(scene);
    ui->graphicsView->show();

    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->setSortingEnabled(false);
    //ui->tableWidget->setColumnWidth(0, 20);
    ui->tableWidget->verticalHeader()->setFixedWidth(0);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showEvent(QShowEvent *event)
{
    QMainWindow::showEvent(event);
    if (!window_activated_flag) {
        window_activated_flag = true;
        setConnections();
        setParameters();
    }
}

void MainWindow::setConnections()
{
    QTimer *_ptrTimerDelayingCall = new QTimer(this);
    _ptrTimerDelayingCall->setSingleShot(true);
    _ptrTimerDelayingCall->setInterval(800);
    connect(_ptrTimerDelayingCall, &QTimer::timeout, this, [this]() {
        renewFolder(input_folder);
    });

    connect(ui->actionAddFolder, &QAction::triggered, this, &MainWindow::onAddFolderClicked);
    connect(ui->actionSetOutputFolder, &QAction::triggered, this, &MainWindow::setOutputFolder);

    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-library-add.png"),
                           tr("Add folder"), this, &MainWindow::onAddFolderClicked);
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-loop-circular.png"),
                           tr("Renew"), this, [this]() {
        renewFolder(input_folder);
    });
    ui->toolBar->addSeparator();   
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-chevron-left.png"),
                           tr("Previous"), this, [this]() {
        scrollImages(Direction::PREVIOUS);
    });
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-chevron-right.png"),
                           tr("Next"), this, [this]() {
        scrollImages(Direction::NEXT);
    });
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-share-boxed.png"),
                           tr("Export Images"), this, &MainWindow::onSaveCollectionClicked);
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-cloud-upload.png"),
                           tr("Export Json"), this, &MainWindow::onSaveJsonClicked);

    connect(ui->buttonInputFolder, &QPushButton::clicked, this, &MainWindow::onAddFolderClicked);
    connect(ui->buttonOutputFolder, &QPushButton::clicked, this, &MainWindow::setOutputFolder);
    connect(ui->buttonLockAR, &QPushButton::clicked, this, &MainWindow::onLockARClicked);
    connect(ui->spinBox_count, &QSpinBox::textChanged, this, [_ptrTimerDelayingCall]() {
        _ptrTimerDelayingCall->stop();
        _ptrTimerDelayingCall->start();
    });
    connect(ui->spinBox_width, &QSpinBox::textChanged, this, [this]() {
        if (lock_ar_flag) {
            const int _width = ui->spinBox_width->value();
            ui->spinBox_height->blockSignals(true);
            ui->spinBox_height->setValue(static_cast<int>(round(aspectRatio*_width)));
            ui->spinBox_height->blockSignals(false);
        }
    });
    connect(ui->spinBox_height, &QSpinBox::textChanged, this, [this]() {
        if (lock_ar_flag) {
            const int _height = ui->spinBox_height->value();
            ui->spinBox_width->blockSignals(true);
            ui->spinBox_width->setValue(static_cast<int>(round(_height/aspectRatio)));
            ui->spinBox_width->blockSignals(false);
        }
    });
    connect(generator, &Generator::progressChanged, this, [this](const int value, const int total) {
        const float percent = 100.f*(static_cast<float>(value)/total);
        ptr_progress->setText(QString::number(value));
        ptr_progress->setPercent(static_cast<int>(percent));
    });
}

void MainWindow::setParameters()
{
    QFile file(":/resources/css/style_1.css");
    if (file.open(QIODevice::ReadOnly)) {
        const QString list = QString::fromUtf8(file.readAll());
        this->setStyleSheet(styleCreator(list));
        file.close();
    }

    SETTINGS(settings);
    settings.beginGroup("Window");
    this->restoreGeometry(settings.value("Window/geometry").toByteArray());
    this->restoreState(settings.value("Window/state").toByteArray());
    settings.endGroup();
    settings.beginGroup("Main");
    lock_ar_flag = settings.value("Main/lock_ar", true).toBool();
    input_folder = settings.value("Main/input_folder").toString();
    output_folder = settings.value("Main/output_folder", QDir::homePath()).toString();
    settings.endGroup();

    ui->lineEdit_outputFolder->setText(output_folder);
    ui->buttonLockAR->setProperty("lock", lock_ar_flag);
    ui->buttonLockAR->style()->polish(ui->buttonLockAR);
    if (!input_folder.isEmpty()) {
        QTimer::singleShot(700, this, [this]() {
            renewFolder(input_folder);
        });
    }
}

void MainWindow::fillLayersTable(const QStringList &fullPaths)
{
    ui->tableWidget->setRowCount(0);
    foreach (const QString &fullPath, fullPaths) {
        const QString path = QFileInfo(fullPath).baseName();
        const int sep = path.indexOf('#');
        const int rowsCount = ui->tableWidget->rowCount();
        ui->tableWidget->setRowCount(rowsCount + 1);
        QTableWidgetItem *path_item = new QTableWidgetItem(path.mid(0, sep));
        path_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        ui->tableWidget->setItem(rowsCount, 0, path_item);
    }
}

void MainWindow::fillValuesTable(const QList<NumPaths> &numFiles)
{
    int index = 0;
    foreach (const NumPaths &numFile, numFiles) {
        const QString file = QFileInfo(numFile.fullPath).baseName();
        const int sep = file.indexOf('#');
        const int chance = numFile.num;
        QTableWidgetItem *value_item = new QTableWidgetItem(file.mid(0, sep));
        QTableWidgetItem *num_item = new QTableWidgetItem(QString::number(chance));
        value_item->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
        num_item->setTextAlignment(Qt::AlignCenter);
        ui->tableWidget->setItem(index, 1, value_item);
        ui->tableWidget->setItem(index, 2, num_item);
        index++;
    }
}

void MainWindow::showScene(const QList<NumPaths> &numFiles)
{
    scene->clear();
    foreach (const NumPaths &numFile, numFiles) {
        QPixmap pixmap(numFile.fullPath);
        if (!folder_added_flag) {
            folder_added_flag = true;
            const int max_width = pixmap.width();
            const int max_height = pixmap.height();
            aspectRatio = static_cast<float>(max_width)/max_height;
            ui->spinBox_width->blockSignals(true);
            ui->spinBox_height->blockSignals(true);
            ui->spinBox_width->setMaximum(max_width);
            ui->spinBox_height->setMaximum(max_height);
            ui->spinBox_width->setValue(max_width);
            ui->spinBox_height->setValue(max_height);
            ui->spinBox_width->blockSignals(false);
            ui->spinBox_height->blockSignals(false);
        }
        const int _width = ui->spinBox_width->value();
        const int _height = ui->spinBox_height->value();
        const QPixmap scaled = pixmap.scaled(_width, _height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
        QGraphicsPixmapItem *pixmapItem = scene->addPixmap(scaled);
        pixmapItem->setTransformationMode(Qt::TransformationMode::FastTransformation);
    }
    ui->graphicsView->fitInView(scene->sceneRect(), Qt::KeepAspectRatio);
    scene->setSceneRect(scene->itemsBoundingRect());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    SETTINGS(settings);
    settings.beginGroup("Window");
    settings.setValue("Window/geometry", this->saveGeometry());
    settings.setValue("Window/state", this->saveState());
    settings.endGroup();
    settings.beginGroup("Main");
    settings.setValue("Main/lock_ar", lock_ar_flag);
    settings.setValue("Main/input_folder", input_folder);
    settings.setValue("Main/output_folder", output_folder);
    settings.endGroup();

    event->accept();
}

QString MainWindow::styleCreator(const QString &list)
{
    QString style = list;
    QStringList varDetect;
    QStringList splitList;
    QStringList varNames;
    QStringList varValues;
    splitList << list.split(';');
    for (int i = 0; i < splitList.size(); i++) {
        if (splitList[i].indexOf("@") != -1 && splitList[i].indexOf("=") != -1) {
            varDetect.append(splitList[i]);
        }
    }
    for (int i = 0; i < varDetect.size(); i++) {
        varNames.append(varDetect[i].split('=')[0].remove(" ").remove("\n"));
        varValues.append(varDetect[i].split('=')[1].remove(" ").remove("\n"));
        style = style.remove(varDetect[i] + QString(";"));
    }
    for (int i = 0; i < varNames.size(); i++) {
        style = style.replace(varNames[i], varValues[i]);
    }
    return style;
}

bool MainWindow::eventFilter(QObject* object, QEvent* event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent*>(event);
        if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
            ui->tableWidget->setFocus();
            return true;
        } else
        if (keyEvent->key() == Qt::Key_Left) {
            scrollImages(Direction::PREVIOUS);
            return true;
        } else
        if (keyEvent->key() == Qt::Key_Right) {
            scrollImages(Direction::NEXT);
            return true;
        }
    } else
    if (event->type() == QEvent::Wheel) {
        if (object == ui->graphicsView) {
            const QGraphicsView::ViewportAnchor anchor = ui->graphicsView->transformationAnchor();
            ui->graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
            QWheelEvent *whellEvent = static_cast<QWheelEvent*>(event);
            const int angle = whellEvent->angleDelta().y();
            qreal factor = (angle > 0) ? 1.1:0.9;
            ui->graphicsView->scale(factor, factor);
            ui->graphicsView->setTransformationAnchor(anchor);
            return QMainWindow::eventFilter(object, event);
        } else
        if (object == ui->graphicsView->verticalScrollBar() ||
            object == ui->graphicsView->horizontalScrollBar()) {
            return true;
        }
    }
    return QMainWindow::eventFilter(object, event);
}

void MainWindow::onAddFolderClicked()
{
    const QString folder = openFolder(tr("Add folder"));
    if (!folder.isEmpty()) {
        folder_added_flag = false;
        renewFolder(folder);
    }
}

void MainWindow::onSaveCollectionClicked()
{
    if (!generatedPathsList.isEmpty() && QDir(output_folder).exists()) {
        const QString images_folder = output_folder + QString("/images");
        if (QDir(images_folder).exists()) {
            QDir(images_folder).removeRecursively();
        }
        if (!QDir(images_folder).exists()) {
            QDir().mkdir(images_folder);
        }
        bool save_aborted = false;
        auto abortSave = [&save_aborted]() {
            save_aborted = true;
        };
        ptr_progress->show();
        ptr_progress->setType(this, ProgressMode::SAVE);
        connect(ptr_progress, &ProgressMessage::saveAborted, this, abortSave);
        ptr_progress->setPercent(0);
        QApplication::processEvents();

        int index = ui->spinBox_startFrom->value();
        foreach (const QList<NumPaths> &numFiles, generatedPathsList) {
            showScene(numFiles);
            scene->clearSelection();
            scene->setSceneRect(scene->itemsBoundingRect());
            QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            scene->render(&painter);
            const QString fileName = images_folder + QString("/%1.png").arg(QString::number(index));
            ptr_progress->setText(fileName);
            ptr_progress->setPercent(50);
            QApplication::processEvents();
            if (save_aborted) break;
            image.save(fileName, "PNG", 20);
            ptr_progress->setPercent(100);
            QApplication::processEvents();
    #if defined (Q_OS_UNIX)
            usleep(50000);
    #elif defined (Q_OS_WIN64)
            Sleep(50);
    #endif
            ptr_progress->setPercent(0);
            index++;
        }
        ptr_progress->hide();
        ptr_progress->disconnect();
    }
}

void MainWindow::setOutputFolder()
{
    const QString folder = openFolder(tr("Output folder"));
    if (!folder.isEmpty()) {
        output_folder = folder;
        ui->lineEdit_outputFolder->setText(folder);
    }
}

void MainWindow::onLockARClicked()
{
    lock_ar_flag = (lock_ar_flag) ? false : true;
    ui->buttonLockAR->setProperty("lock", lock_ar_flag);
    ui->buttonLockAR->style()->polish(ui->buttonLockAR);
}

QString MainWindow::openFolder(const QString &title)
{
    QFileDialog selectFolderWindow(nullptr);
    selectFolderWindow.setWindowTitle(title);
    selectFolderWindow.setMinimumWidth(600);
    selectFolderWindow.setWindowFlags(Qt::Dialog | Qt::SubWindow);
#if defined (Q_OS_UNIX)
    selectFolderWindow.setOption(QFileDialog::DontUseNativeDialog, true);
#endif
    selectFolderWindow.setFileMode(QFileDialog::DirectoryOnly);
    selectFolderWindow.setAcceptMode(QFileDialog::AcceptOpen);
    selectFolderWindow.setDirectory(QDir::homePath());
    if (selectFolderWindow.exec() == QFileDialog::Accepted) {
        return selectFolderWindow.selectedFiles().at(0);
    }
    return QString("");
}

void MainWindow::showMessage(const QString &message)
{
    Message msg(this);
    msg.setMessage(message);
    msg.setModal(true);
    msg.exec();
}

void MainWindow::renewFolder(const QString &folder)
{
    QList<NumPaths> numPaths = generator->getFilesOrFolders(folder, TypeOfObject::FOLDERS);
    if (numPaths.isEmpty()) {
        showMessage(tr("Folder is empty or set the sequence of layers first!"));
        return;
    }
    int maxPossibleCount = 1;
    QStringList paths;
    bool correctSequenceFlag = true;
    QList<QList<NumPaths>> totalPathsList;
    foreach (const NumPaths &numPath, numPaths) {
        paths << numPath.fullPath;
        QList<NumPaths> numFilesInPath = generator->getFilesOrFolders(numPath.fullPath, TypeOfObject::FILES);
        if (numFilesInPath.isEmpty()) {
            totalPathsList = QList<QList<NumPaths>>();
            correctSequenceFlag = false;
            showMessage(tr("First set the weight of the images in the folder: %1").arg(numPath.fullPath));
            break;
        } else {
            totalPathsList << numFilesInPath;
            maxPossibleCount *= numFilesInPath.size();
        }
    }
    if (correctSequenceFlag) {
        input_folder = folder;
        current_image = 0;

        ptr_progress->show();
        ptr_progress->setType(this, ProgressMode::GENERATE);
        QApplication::processEvents();

        int generatedCount = ui->spinBox_count->value();
        generatedPathsList = generator->generateCollection(generatedCount, totalPathsList);

        if (!generatedPathsList.isEmpty()) {  
            ui->lineEdit_inputFolder->setText(folder);
            fillLayersTable(paths);
            fillValuesTable(generatedPathsList.at(0));
            ui->lineEdit_currentImage->setText(QString::number(current_image + 1));
            showScene(generatedPathsList.at(0));
        }

        QString count = QString::number(maxPossibleCount);
        int w_size = count.size();
        if (w_size > 3) count.insert(count.size() - 3, " ");
        if (w_size > 6) count.insert(count.size() - 7, " ");
        statusBar()->showMessage(tr("Maximum possible combinations count: %1").arg(count));
        ptr_progress->hide();
    }
}

void MainWindow::scrollImages(const int direction)
{
    auto fillWidgets = [this](const int index){
        ui->lineEdit_currentImage->setText(QString::number(index + 1));
        fillValuesTable(generatedPathsList.at(index));
        showScene(generatedPathsList.at(index));
    };
    if (current_image > 0 && generatedPathsList.size() >= current_image && direction == Direction::PREVIOUS) {
        current_image--;
        fillWidgets(current_image);
    } else
    if (generatedPathsList.size() > current_image + 1 && direction == Direction::NEXT) {
        current_image++;
        fillWidgets(current_image);
    }
}

void MainWindow::onSaveJsonClicked()
{
    if (!generatedPathsList.isEmpty() && QDir(output_folder).exists()) {
        const QString json_folder = output_folder + QString("/json");
        if (QDir(json_folder).exists()) {
            QDir(json_folder).removeRecursively();
        }
        if (!QDir(json_folder).exists()) {
            QDir().mkdir(json_folder);
        }
        bool save_aborted = false;
        auto abortSave = [&save_aborted]() {
            save_aborted = true;
        };
        ptr_progress->show();
        ptr_progress->setType(this, ProgressMode::SAVE);
        connect(ptr_progress, &ProgressMessage::saveAborted, this, abortSave);
        ptr_progress->setPercent(0);
        QApplication::processEvents();

        int index = ui->spinBox_startFrom->value();
        foreach (const QList<NumPaths> &numFiles, generatedPathsList) {
            const qint64 _date = QDateTime::currentMSecsSinceEpoch();

            quint32 value = QRandomGenerator::global()->generate();
            QTime time = QTime::currentTime();
            qsrand((uint)time.msec());
            QString randomHex;
            for (int i = 0; i < 40; i++) {
                const int n = qrand() % 16;
                randomHex.append(QString::number(n, 16));
            }


            QJsonObject obj;
            obj["name"] = ui->lineEdit_name->text();
            obj["description"] = ui->textEdit_description->toPlainText().replace("\n", " ");
            obj["image"] = ui->lineEdit_uri->text();
            obj["dna"] = randomHex;
            obj["edition"] = index;
            obj["date"] = _date;

            QJsonArray array;
            QList<QString> list = {"ddd", "ggg"};
            foreach (const QString level, list) {
                QJsonObject levelObject;
                levelObject["trait_type"] = "rrr";
                levelObject["value"] = "rrr";
                array.append(levelObject);
            }
            obj["attributes"] = array;

            QJsonDocument doc(obj);


            const QString fileName = json_folder + QString("/%1.json").arg(QString::number(index));
            ptr_progress->setText(fileName);
            ptr_progress->setPercent(50);
            QApplication::processEvents();
            if (save_aborted) break;

            QFile jsonFile(fileName);
            if (!jsonFile.open(QIODevice::WriteOnly)) {
                qWarning("Couldn't open save file.");
                break;
            }
            jsonFile.write(doc.toJson());
            jsonFile.close();

            ptr_progress->setPercent(100);
            QApplication::processEvents();
    #if defined (Q_OS_UNIX)
            usleep(50000);
    #elif defined (Q_OS_WIN64)
            Sleep(50);
    #endif
            ptr_progress->setPercent(0);
            index++;
        }
        ptr_progress->hide();
        ptr_progress->disconnect();
    }
}
