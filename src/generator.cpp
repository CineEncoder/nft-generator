#include "generator.h"

Generator::Generator(QObject *parent):
    QObject(parent)
{

}

Generator::~Generator()
{

}

QList<QList<NumPaths>> Generator::generateCollection(const int generatedCount,
                                                     const QList<QList<NumPaths>> &totalPathsList)
{
    if (totalPathsList.isEmpty()) {
        // Unexpected error
        return QList<QList<NumPaths>>();
    }
#if defined (Q_OS_WIN)
    #ifdef __MINGW64__
        std::mt19937 gen(time(nullptr));
    #else
        std::random_device rd;
        std::mt19937 gen(rd());
    #endif
#elif defined(Q_OS_UNIX)
    std::random_device rd;
    std::mt19937 gen(rd());
#endif

    auto generate = [&totalPathsList, &gen]()->QList<NumPaths> {
        QList<NumPaths> sample;
        foreach (const QList<NumPaths> &numFiles, totalPathsList) {
            int summ = 0;
            foreach (const NumPaths &numFile, numFiles) {
                summ += numFile.num;
                //cout << "File: " << numFile.fullPath.toStdString() << std::endl;
            }
            QVector<double> weights;
            foreach (const NumPaths &numFile, numFiles) {
                weights.push_back(static_cast<double>(numFile.num)/summ);
            }           
            std::discrete_distribution<int> dist(std::begin(weights), std::end(weights));
            int pos = dist(gen);
            sample << numFiles.at(pos);
            //cout << "Selected: " << numFiles.at(pos).fullPath.toStdString() << std::endl;
            /*std::map<int, int> map;
            for (int n=0; n<10000; ++n) {
                ++map[dist(gen)];
            }
            for (const auto [num, count] : map) {
                std::cout << num << " generated " << std::setw(4) << count << " times\n";
            }*/
        }
        return sample;
    };

    QList<QList<NumPaths>> generatedPathsList;
    auto search_element = [&generatedPathsList](QList<NumPaths> &sample)->bool {
        foreach (const QList<NumPaths> &numFiles, generatedPathsList) {
            if (numFiles.size() == sample.size()) {
                bool flag = true;
                for (int i = 0; i < sample.size(); i++) {
                    if (numFiles.at(i).fullPath != sample.at(i).fullPath) {
                        flag = false;
                        break;
                    }
                }
                if (flag) return true;
            }
        }
        return false;
    };

    for (int i = 1; i <= generatedCount; i++) {
        for (int j = 0; j < 1000; j++) {
            QApplication::processEvents();
            QList<NumPaths> sample = generate();
            if (!search_element(sample)) {
                generatedPathsList << sample;
                break;
            } else {
                cout << "Repeat... " << std::endl;
            }
        }
        emit progressChanged(i, generatedCount);
    }
    return generatedPathsList;
}

QList<NumPaths> Generator::getFilesOrFolders(const QString &folder, const int typeOfObject)
{
    auto numLessThan = [](const NumPaths &a, const NumPaths &b)->bool
    {
        return a.num < b.num;
    };
    QList<NumPaths> numPaths;
    bool correctSequenceFlag = true;
    QDirIterator *it = (typeOfObject == TypeOfObject::FILES) ? new QDirIterator(folder, {"*.png", "*.PNG"}, QDir::Files) :
        new QDirIterator(folder, QDir::Dirs | QDir::NoDot | QDir::NoDotAndDotDot);
    while (it->hasNext()) {
        const QString fullPath = it->next();
        const QString path = QFileInfo(fullPath).baseName();
        const int sep = path.indexOf('#') + 1;
        const int num = path.midRef(sep).toInt();
        NumPaths numPath;
        numPath.num = num;
        numPath.fullPath = fullPath;
        numPaths << numPath;
        if (num == 0) {
            correctSequenceFlag = false;
            break;
        }
    }
    delete it;
    if (correctSequenceFlag) {
        sort(numPaths.begin(), numPaths.end(), numLessThan);
        return numPaths;
    }
    return QList<NumPaths>();
}
