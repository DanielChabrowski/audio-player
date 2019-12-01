#include "FileUtilities.hpp"

#include <QFile>
#include <QFileInfo>

QString getUniqueFilename(const QString &filename)
{
    if(not QFile::exists(filename))
    {
        return filename;
    }

    QFileInfo fileInfo{ filename };
    const QString suffix = '.' + fileInfo.completeSuffix();
    const QString filepath = fileInfo.path() + '/' + fileInfo.baseName();

    QString ret;
    for(int i = 1;; ++i)
    {
        ret = QString("%1 (%2)%3").arg(filepath).arg(i).arg(suffix);
        if(not QFile::exists(ret))
        {
            return ret;
        }
    }
}
