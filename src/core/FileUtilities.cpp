#include "FileUtilities.hpp"

#include <QFileInfo>

QString getUniqueFilename(const QString &filename)
{
    if(not QFileInfo::exists(filename))
    {
        return filename;
    }

    const QFileInfo fileInfo{ filename };

    QString suffix = fileInfo.completeSuffix();
    if(not suffix.isEmpty())
    {
        suffix.prepend('.');
    }

    const QString filepath = fileInfo.path() + '/' + fileInfo.baseName();

    QString ret;
    for(int i = 1;; ++i)
    {
        ret = QString("%1 (%2)%3").arg(filepath).arg(i).arg(suffix);
        if(not QFileInfo::exists(ret))
        {
            return ret;
        }
    }
}
