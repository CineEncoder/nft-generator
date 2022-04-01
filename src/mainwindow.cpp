#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "message.h"
#include "dialog.h"
//#include <QDebug>

MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    layers(QList<QString>()),
    generatedPathsList(QList<QList<NumPaths>>()),
    ui(new Ui::MainWindow),
    window_activated_flag(false),
    lock_ar_flag(true),
    folder_added_flag(false),
    current_image(0),
    maxPossibleCount(0),
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
    setContentsMargins(4,4,4,4);
}

MainWindow::~MainWindow()
{
    delete ui;
}

QDataStream& operator<<(QDataStream &out, const NumPaths &path)
{
    out << path.num;
    out << path.fullPath;
    return out;
}

QDataStream& operator>>(QDataStream &in, NumPaths &path)
{
    in >> path.num;
    in >> path.fullPath;
    return in;
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

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    SETTINGS(settings);
    settings.beginGroup("Window");
    settings.setValue("Window/geometry", this->saveGeometry());
    settings.setValue("Window/state", this->saveState());
    settings.setValue("Window/theme", theme);
    settings.endGroup();
    settings.beginGroup("Main");
    settings.setValue("Main/lock_ar", lock_ar_flag);
    settings.setValue("Main/input_folder", input_folder);
    settings.setValue("Main/output_folder", output_folder);
    settings.setValue("Main/project_folder", project_folder);
    settings.endGroup();

    event->accept();
}

void MainWindow::setConnections()
{
    QTimer *_ptrTimerDelayingCall = new QTimer(this);
    _ptrTimerDelayingCall->setSingleShot(true);
    _ptrTimerDelayingCall->setInterval(800);
    connect(_ptrTimerDelayingCall, &QTimer::timeout, this, [this]() {
        regenerateCollection(input_folder);
    });
    //================= File menu ================
    connect(ui->actionAddFolder, &QAction::triggered, this, &MainWindow::onAddFolderClicked);
    connect(ui->actionSetOutputFolder, &QAction::triggered, this, &MainWindow::setOutputFolder);
    connect(ui->actionSaveProjectAs, &QAction::triggered, this, &MainWindow::onSaveProjectAsClicked);
    connect(ui->actionLoadProject, &QAction::triggered, this, &MainWindow::onLoadProjectClicked);
    connect(ui->actionExportImages, &QAction::triggered, this, &MainWindow::onSaveCollectionClicked);
    connect(ui->actionExportJson, &QAction::triggered, this, &MainWindow::onSaveJsonClicked);
    //================== Edit menu ===============
    connect(ui->actionPrevious, &QAction::triggered, this, [this]() {
        scrollImages(Direction::PREVIOUS);
    });
    connect(ui->actionNext, &QAction::triggered, this, [this]() {
        scrollImages(Direction::NEXT);
    });
    //================== View menu ===============
    QList<QDockWidget*> dockList = findChildren<QDockWidget*>();
    foreach (QDockWidget *dock, dockList) {
        ui->menuView->addAction(dock->toggleViewAction());
    }
    //============================================

    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-library-add.png"),
                           tr("Add folder"), this, &MainWindow::onAddFolderClicked);
    ui->toolBar->addAction(QIcon(":/resources/icons/16x16/cil-loop-circular.png"),
                           tr("Renew"), this, [this]() {
        regenerateCollection(input_folder);
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
    connect(ui->spinBox_count, &QSpinBox::textChanged, this, [this, _ptrTimerDelayingCall]() {
        const int value = ui->spinBox_count->value();
        ui->spinBox_endInt->setMaximum(value);
        ui->spinBox_endInt->setValue(value);
        _ptrTimerDelayingCall->stop();
        _ptrTimerDelayingCall->start();
    });
    connect(ui->spinBox_endInt, &QSpinBox::textChanged, this, [this]() {
        ui->spinBox_startInt->setMaximum(ui->spinBox_endInt->value());
    });
    connect(ui->spinBox_width, &QSpinBox::textChanged, this, [this]() {
        if (lock_ar_flag) {
            const float _width = static_cast<float>(ui->spinBox_width->value());
            ui->spinBox_height->blockSignals(true);
            ui->spinBox_height->setValue(static_cast<int>(round(_width/aspectRatio)));
            ui->spinBox_height->blockSignals(false);
        }
    });
    connect(ui->spinBox_height, &QSpinBox::textChanged, this, [this]() {
        if (lock_ar_flag) {
            const int _height = ui->spinBox_height->value();
            ui->spinBox_width->blockSignals(true);
            ui->spinBox_width->setValue(static_cast<int>(round(aspectRatio*_height)));
            ui->spinBox_width->blockSignals(false);
        }
    });
    connect(generator, &Generator::progressChanged, this, [this](const int value, const int total) {
        const float percent = 100.f*(static_cast<float>(value)/total);
        ptr_progress->setText(QString::number(value));
        ptr_progress->setPercent(static_cast<int>(percent));
    });
    connect(ui->buttonAddMetadata, &QPushButton::clicked, this, [this]() {
        const int rowsCount = ui->tableWidget_advanced->rowCount();
        ui->tableWidget_advanced->setRowCount(rowsCount + 1);
        QTableWidgetItem *key_item = new QTableWidgetItem(QString("key %1").arg(QString::number(rowsCount + 1)));
        QTableWidgetItem *value_item = new QTableWidgetItem(QString("value %1").arg(QString::number(rowsCount + 1)));
        ui->tableWidget_advanced->setItem(rowsCount, 0, key_item);
        ui->tableWidget_advanced->setItem(rowsCount, 1, value_item);
    });
    connect(ui->buttonRemoveMetadata, &QPushButton::clicked, this, [this]() {
        const int row = ui->tableWidget_advanced->currentRow();
        if (row != -1) ui->tableWidget_advanced->removeRow(row);
    });
}

void MainWindow::setParameters()
{
    SETTINGS(settings);
    settings.beginGroup("Window");
    this->restoreGeometry(settings.value("Window/geometry").toByteArray());
    this->restoreState(settings.value("Window/state").toByteArray());
    theme = settings.value("Main/theme", Theme::DEFAULT).toInt();
    settings.endGroup();
    settings.beginGroup("Main");
    lock_ar_flag = settings.value("Main/lock_ar", true).toBool();
    input_folder = settings.value("Main/input_folder").toString();
    output_folder = settings.value("Main/output_folder", QDir::homePath()).toString();
    project_folder = settings.value("Main/project_folder", QDir::homePath()).toString();
    settings.endGroup();

    ui->lineEdit_inputFolder->setText(input_folder);
    ui->lineEdit_outputFolder->setText(output_folder);
    ui->buttonLockAR->setProperty("lock", lock_ar_flag);
    ui->buttonLockAR->style()->polish(ui->buttonLockAR);
    setTheme(Theme::DARK);
    /*if (!input_folder.isEmpty()) {
        QTimer::singleShot(700, this, [this]() {
            regenerateCollection(input_folder);
        });
    }*/
}

void MainWindow::fillLayersTable(const QStringList &fullPaths)
{
    ui->tableWidget->setRowCount(0);
    layers = QList<QString>();
    foreach (const QString &fullPath, fullPaths) {
        const QString path = QFileInfo(fullPath).baseName();
        const int sep = path.indexOf('#');
        const int rowsCount = ui->tableWidget->rowCount();
        const QString layerName = path.mid(0, sep);
        layers << layerName;
        ui->tableWidget->setRowCount(rowsCount + 1);
        QTableWidgetItem *path_item = new QTableWidgetItem(layerName);
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

QString MainWindow::styleCreator(const QString &list)
{
    QString style = list;
    QStringList splitList;
    QStringList varList;
    QStringList varNames;
    QStringList varValues;
    splitList << list.split(';');

    foreach (const QString &row, splitList) {
        const int first_symbol = row.indexOf('@');
        if (first_symbol != -1 && row.indexOf('=') != -1) {
            varList.append(row.mid(first_symbol));
        }
    }
    foreach (const QString &var, varList) {
        varNames.append(var.split('=')[0].remove(' ').remove('\n'));
        varValues.append(var.split('=')[1].remove(' ').remove('\n'));
        style = style.remove(var + QString(";"));
    }
    for (int i = 0; i < varNames.size() && i < varValues.size(); i++) {
        style = style.replace(varNames[i], varValues[i]);
    }
    //std::cout << style.toStdString() << std::endl;
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
    const QString folder = openFileDialog(tr("Add folder"), FileDialogType::OPENFOLDER);
    if (!folder.isEmpty()) {
        regenerateCollection(folder);
    }
}

void MainWindow::setOutputFolder()
{
    const QString folder = openFileDialog(tr("Output folder"), FileDialogType::OPENFOLDER);
    if (!folder.isEmpty()) {
        output_folder = folder;
        ui->lineEdit_outputFolder->setText(folder);
    }
}

QString MainWindow::openFileDialog(const QString &title, int dialogType)
{
    QFileDialog fileDialog(nullptr);
    fileDialog.setWindowTitle(title);
    fileDialog.setMinimumWidth(600);
    fileDialog.setWindowFlags(Qt::Dialog | Qt::SubWindow);
#if defined (Q_OS_UNIX)
    fileDialog.setOptions(QFileDialog::DontUseNativeDialog);
#endif
    fileDialog.setOptions(QFileDialog::DontResolveSymlinks);
    if (dialogType == FileDialogType::LOADFILE) {
        fileDialog.setDirectory(project_folder);
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
        fileDialog.setFileMode(QFileDialog::ExistingFile);
        fileDialog.setNameFilter(tr("Project files: *.nft (*.nft)"));
    } else
    if (dialogType == FileDialogType::SAVEFILE) {
        fileDialog.setDirectory(project_folder);
        fileDialog.setAcceptMode(QFileDialog::AcceptSave);
        fileDialog.selectFile(QString("untitled.nft"));
        fileDialog.setNameFilter("Project file (*.nft)");
    } else
    if (dialogType == FileDialogType::OPENFOLDER) {
        fileDialog.setDirectory(QDir::homePath());
        fileDialog.setFileMode(QFileDialog::DirectoryOnly);
        fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
    }
    if (fileDialog.exec() == QFileDialog::Accepted) {
        if (dialogType == FileDialogType::LOADFILE || dialogType == FileDialogType::SAVEFILE) {
            project_folder = QFileInfo(fileDialog.selectedFiles().at(0)).absolutePath();
        }
        return fileDialog.selectedFiles().at(0);
    }
    return QString("");
}

void MainWindow::onLockARClicked()
{
    lock_ar_flag = (lock_ar_flag) ? false : true;
    ui->buttonLockAR->setProperty("lock", lock_ar_flag);
    ui->buttonLockAR->style()->polish(ui->buttonLockAR);
}

void MainWindow::showMessage(const QString &message)
{
    Message msg(this);
    msg.setMessage(message);
    msg.setModal(true);
    msg.exec();
}

void MainWindow::renewFolder(const QString &folder,
                             QList<QList<NumPaths>> &totalPathsList,
                             QStringList &paths,
                             bool &correctSequenceFlag)
{
    QList<NumPaths> numPaths = generator->getFilesOrFolders(folder, TypeOfObject::FOLDERS);
    totalPathsList = QList<QList<NumPaths>>();
    paths = QStringList();
    if (numPaths.isEmpty()) {
        showMessage(tr("Folder is empty or set the sequence of layers first!"));
        correctSequenceFlag = false;
        return;
    }
    maxPossibleCount = 1;
    folder_added_flag = false;
    correctSequenceFlag = true;
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
}

void MainWindow::renewData(const QString &folder, const QStringList &paths)
{
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
}

void MainWindow::regenerateCollection(const QString &folder)
{
    bool correctSequenceFlag = true;
    QList<QList<NumPaths>> totalPathsList;
    QStringList paths;
    renewFolder(folder, totalPathsList, paths, correctSequenceFlag);
    if (correctSequenceFlag) {
        input_folder = folder;
        current_image = 0;
        ui->spinBox_count->blockSignals(true);
        ui->spinBox_count->setMaximum((maxPossibleCount > 50000) ? 50000:maxPossibleCount);
        ui->spinBox_count->blockSignals(false);
        ptr_progress->show();
        ptr_progress->setType(this, ProgressMode::GENERATE);
        QApplication::processEvents();
        int generatedCount = ui->spinBox_count->value();
        generatedPathsList = generator->generateCollection(generatedCount, totalPathsList);
        renewData(folder, paths);
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

void MainWindow::setTheme(const int theme)
{
    QString themePath = QString(":/resources/css/style_%1.css")
            .arg(QString::number(theme));
    QFile file(themePath);
    if (file.open(QIODevice::ReadOnly)) {
        const QString list = QString::fromUtf8(file.readAll());
        this->setStyleSheet(styleCreator(list));
        file.close();
    }
}

void MainWindow::onSaveCollectionClicked()
{
    if (!generatedPathsList.isEmpty() && QDir(output_folder).exists()) {
        const int startInt = ui->spinBox_startInt->value() - 1;
        const int endInt = ui->spinBox_endInt->value();
        if (endInt > generatedPathsList.size()) {
            showMessage(tr("Bad interval to save collection!"));
            return;
        }
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

        int serial = 0;
        const int index = ui->spinBox_startFrom->value();
        const int collectSize = endInt - startInt;
        for (int i = startInt; i < endInt; i++) {
            showScene(generatedPathsList.at(i));
            scene->clearSelection();
            scene->setSceneRect(scene->itemsBoundingRect());
            QImage image(scene->sceneRect().size().toSize(), QImage::Format_ARGB32);
            image.fill(Qt::transparent);
            QPainter painter(&image);
            scene->render(&painter);
            const QString fileName = images_folder + QString("/%1.png").arg(QString::number(serial + index));
            ptr_progress->setText(fileName);
            const float percent = 100.f*(static_cast<float>(serial + 1)/collectSize);
            QApplication::processEvents();
            if (save_aborted) break;
            image.save(fileName, "PNG", 20);
            ptr_progress->setPercent(static_cast<int>(percent));
            QApplication::processEvents();
    #if defined (Q_OS_UNIX)
            usleep(50000);
    #elif defined (Q_OS_WIN64)
            Sleep(50);
    #endif
            serial++;
        }
        ptr_progress->hide();
        ptr_progress->disconnect();
    }
}

void MainWindow::onSaveJsonClicked()
{
    if (!generatedPathsList.isEmpty() && QDir(output_folder).exists()) {
        const int startInt = ui->spinBox_startInt->value() - 1;
        const int endInt = ui->spinBox_endInt->value();
        if (endInt > generatedPathsList.size()) {
            showMessage(tr("Bad interval to save Json!"));
            return;
        }
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

        int serial = 0;
        const int index = ui->spinBox_startFrom->value();
        const int collectSize = endInt - startInt;
        const int rowCount = ui->tableWidget_advanced->rowCount();
        QJsonArray agregateArray;
        for (int i = startInt; i < endInt; i++) {
            const qint64 _date = QDateTime::currentMSecsSinceEpoch();

            quint32 value = QRandomGenerator::global()->generate();
            QTime time = QTime::currentTime();
            qsrand((uint)time.msec());
            QString randomHex;
            for (int i = 0; i < 40; i++) {
                const int n = qrand() % 16;
                randomHex.append(QString::number(n, 16));
            }
            //============== Metadata ===============
            QJsonObject obj;
            obj["name"] = ui->lineEdit_name->text() + QString(" #%1").arg(QString::number(serial + index));
            obj["description"] = ui->textEdit_description->toPlainText().replace("\n", " ");
            obj["image"] = ui->lineEdit_uri->text() + QString("/%1.json").arg(QString::number(serial + index));
            obj["dna"] = randomHex;
            obj["edition"] = serial + index;
            obj["date"] = _date;
            //========= Advanced metadata ===========
            for (int row = 0; row < rowCount; row++) {
                const QString _key = ui->tableWidget_advanced->item(row, 0)->text();
                const QString _value = ui->tableWidget_advanced->item(row, 1)->text();
                obj[_key] = _value;
            }
            //============= Arrtibutes ==============
            QJsonArray array;
            int layerInd = 0;
            if (layers.size() != generatedPathsList.at(i).size()) {
                qWarning("Unexpected error!");
                break;
            }
            foreach (const NumPaths &numFile, generatedPathsList.at(i)) {
                const QString file = QFileInfo(numFile.fullPath).baseName();
                const int sep = file.indexOf('#');
                QJsonObject levelObject;
                levelObject["trait_type"] = layers.at(layerInd);
                levelObject["value"] = file.mid(0, sep);
                array.append(levelObject);
                layerInd++;
            }
            obj["attributes"] = array;
            //========================================
            QJsonDocument doc(obj);
            agregateArray.append(obj);

            const QString fileName = json_folder + QString("/%1.json").arg(QString::number(serial + index));
            ptr_progress->setText(fileName);
            const float percent = 100.f*(static_cast<float>(serial + 1)/collectSize);
            QApplication::processEvents();
            if (save_aborted) break;

            QFile jsonFile(fileName);
            if (!jsonFile.open(QIODevice::WriteOnly)) {
                qWarning("Couldn't open save file.");
                break;
            }
            jsonFile.write(doc.toJson());
            jsonFile.close();

            ptr_progress->setPercent(static_cast<int>(percent));
            QApplication::processEvents();
    #if defined (Q_OS_UNIX)
            usleep(50000);
    #elif defined (Q_OS_WIN64)
            Sleep(50);
    #endif
            serial++;
        }

        QApplication::processEvents();
        const QString agregateFileName = json_folder + QString("/_metadata.json");
        ptr_progress->setText(agregateFileName);
        ptr_progress->setPercent(0);
#if defined (Q_OS_UNIX)
        usleep(50000);
#elif defined (Q_OS_WIN64)
        Sleep(50);
#endif
        QApplication::processEvents();
        ptr_progress->setPercent(50);
        QApplication::processEvents();
        if (!save_aborted) {
            QJsonDocument doc(agregateArray);
            QFile jsonFile(agregateFileName);
            if (jsonFile.open(QIODevice::WriteOnly)) {
                jsonFile.write(doc.toJson());
                jsonFile.close();

                ptr_progress->setPercent(100);
                QApplication::processEvents();

            } else {
                qWarning("Couldn't open save file.");
            }
        }
        ptr_progress->hide();
        ptr_progress->disconnect();
    }
}

void MainWindow::onSaveProjectAsClicked()
{
    if (generatedPathsList.size() > 0) {
        const int rowCount = ui->tableWidget_advanced->rowCount();
        QMap<QString, QString> tableMap;
        for (int row = 0; row < rowCount; row++) {
            const QString key = ui->tableWidget_advanced->item(row, 0)->text();
            const QString value = ui->tableWidget_advanced->item(row, 1)->text();
            tableMap[key] = value;
        }
        QString outFilePath = openFileDialog(tr("Save Project"), FileDialogType::SAVEFILE);
        if (outFilePath.isEmpty()) return;
        QFile savedFile(outFilePath);
        if (savedFile.open(QIODevice::WriteOnly)) {
            QDataStream out(&savedFile);
            out.setVersion(QDataStream::Qt_4_0);
            out << (quint32)0xCEFDEF;
            out << tableMap;
            out << ui->lineEdit_name->text();
            out << ui->textEdit_description->toPlainText();
            out << ui->lineEdit_uri->text();
            out << ui->spinBox_count->value();
            out << ui->spinBox_startFrom->value();
            out << ui->spinBox_startInt->value();
            out << ui->spinBox_endInt->value();
            out << input_folder;
            out << generatedPathsList;
            savedFile.close();
            showMessage(tr("Project saved!"));
        } else {
            showMessage(tr("Can`t save file!"));
        }
    }
    else {
        showMessage(tr("Collection is empty!"));
    }
}

void MainWindow::onLoadProjectClicked()
{
    QString selectedFilePath = openFileDialog(tr("Load Project"), FileDialogType::LOADFILE);
    if (selectedFilePath.isEmpty()) return;
    QFile loadedFile(selectedFilePath);
    if (loadedFile.open(QIODevice::ReadOnly)) {
        QDataStream in(&loadedFile);
        in.setVersion(QDataStream::Qt_4_0);
        quint32 magic;
        in >> magic;
        if (magic != 0xCEFDEF) {
            loadedFile.close();
            showMessage(tr("Incorrect project file!!!"));
            return;
        }
        QList<QList<NumPaths>> _generatedPathsList;
        int     count,
                start_from,
                startInt,
                endInt;
        QString name,
                description,
                uri,
                folder;
        QMap<QString, QString> tableMap;
        in >> tableMap;
        in >> name;
        in >> description;
        in >> uri;
        in >> count;
        in >> start_from;
        in >> startInt;
        in >> endInt;
        in >> folder;
        in >> _generatedPathsList;
        loadedFile.close();
        bool correctSequenceFlag = true;
        QList<QList<NumPaths>> totalPathsList;
        QStringList paths;
        renewFolder(folder, totalPathsList, paths, correctSequenceFlag);
        if (correctSequenceFlag && _generatedPathsList.size() > 0) {
            current_image = 0;
            input_folder = folder;
            ui->tableWidget_advanced->setRowCount(0);
            QList<QString> keys = tableMap.keys();
            for (auto &key: keys) {
                const int rowsCount = ui->tableWidget_advanced->rowCount();
                ui->tableWidget_advanced->setRowCount(rowsCount + 1);
                QTableWidgetItem *key_item = new QTableWidgetItem(key);
                QTableWidgetItem *value_item = new QTableWidgetItem(tableMap[key]);
                ui->tableWidget_advanced->setItem(rowsCount, 0, key_item);
                ui->tableWidget_advanced->setItem(rowsCount, 1, value_item);
            }
            ui->lineEdit_name->setText(name);
            ui->textEdit_description->setText(description);
            ui->lineEdit_uri->setText(uri);

            ui->spinBox_count->blockSignals(true);
            ui->spinBox_count->setValue(count);
            ui->spinBox_count->setMaximum((maxPossibleCount > 50000) ? 50000:maxPossibleCount);
            ui->spinBox_count->blockSignals(false);
            ui->spinBox_startFrom->setValue(start_from);
            ui->spinBox_startInt->setValue(startInt);
            ui->spinBox_endInt->setMaximum(count);
            ui->spinBox_endInt->setValue(endInt);

            ui->lineEdit_inputFolder->setText(input_folder);
            generatedPathsList = _generatedPathsList;
            renewData(folder, paths);
        }
    }
    else {
        showMessage(tr("Cannot open file!!!"));
    }
}

