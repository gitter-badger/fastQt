/*
Copyright Copyright 2016-17 Sacha Schutz

    This file is part of fastQT.

    Foobar is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Foobar is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Foobar.  If not, see <http://www.gnu.org/licenses/>.

    @author : Pierre Lindenbaum from FastQC <http://www.bioinformatics.babraham.ac.uk/projects/fastqc/>
    @author : Sacha Schutz <sacha@labsquare.org>
    @author : Pierre Marijon <pierre@marijon.fr>
*/
#include "analysisrunner.h"


AnalysisRunner::AnalysisRunner(QObject *parent)
    :QThread(parent)
{

}

AnalysisRunner::AnalysisRunner(const QString &filename, QObject *parent)
    :QThread(parent)
{
    setFilename(filename);
}

AnalysisRunner::~AnalysisRunner()
{
    mAnalysisList.clear();
}

void AnalysisRunner::run()
{

    QFileInfo fileInfo(mFilename);
    QIODevice * file = Q_NULLPTR;

    if (fileInfo.suffix() == "gz")
        file = new KCompressionDevice(mFilename, KCompressionDevice::GZip);

    if (fileInfo.suffix() == "bz2")
        file = new KCompressionDevice(mFilename, KCompressionDevice::BZip2);


    if (fileInfo.suffix() == "xz")
        file = new KCompressionDevice(mFilename, KCompressionDevice::Xz);

    if (fileInfo.suffix() == "fastq")
        file = new QFile(mFilename);

    if (file == Q_NULLPTR)
    {
        qDebug()<<Q_FUNC_INFO<<fileInfo.suffix()<< " file is not supported";
        return;
    }

    if (file->open(QIODevice::ReadOnly))
    {
        mSequenceCount = 0;
        mProgression   = 0;


        FastqReader reader(file);

        // pre compute total size for sequencial access .
        emitUpdate(tr("Analysis ..."));
        reader.computeTotalSize();

        QTime start = QTime::currentTime();
        qDebug()<<"start"<<start;

        while (reader.next())
        {

            // check if first sequence is valid..Means it's probably a good file
            if (mSequenceCount == 0)
            {
                if (!reader.sequence().isValid())
                {
                    qCritical()<<Q_FUNC_INFO<<"Cannot read sequence. Are you sure it's a Fastq file ?";
                    emitUpdate("Cannot read file");
                    return ;
                }
            }


            ++mSequenceCount;
            // this is critcal and can decrease the speed. Send message only 1 sequence / 1000
            if (mSequenceCount % 1000 == 0)
            {
                int percentNow = reader.percentComplete();
                // if percentNow is still null, return empty percent ...
                if ( (percentNow >= mProgression + 5) || (percentNow == 0))
                {
                    mProgression = percentNow;
                    emitUpdate(QString(tr("%1 Sequences procceed ( %2 \% )")).arg(mSequenceCount).arg(mProgression));
                }

            }


            for (Analysis * a : mAnalysisList)
            {
                a->processSequence(reader.sequence());
            }
        }

        mProgression = 100;
        emitUpdate(tr("Complete in %1 sec").arg(start.msecsTo(QTime::currentTime())));

        qDebug()<<"end"<<start.msecsTo(QTime::currentTime());

    }

    file->close();
    delete file;


}

void AnalysisRunner::addAnalysis(Analysis *analysis)
{
    mAnalysisList.append(analysis);
}

void AnalysisRunner::setFilename(const QString &filename)
{
    mFilename = filename;
}

void AnalysisRunner::reset()
{
    for (Analysis * a : analysisList())
        a->reset();
}

const QString &AnalysisRunner::filename() const
{
    return mFilename;
}

int AnalysisRunner::progression() const
{
    return mProgression;
}

int AnalysisRunner::sequenceCount() const
{
    return mSequenceCount;
}

const QString &AnalysisRunner::lastMessage() const
{
    return mMessage;
}

const QVector<Analysis*> &AnalysisRunner::analysisList() const
{
    return mAnalysisList;
}

void AnalysisRunner::emitUpdate(const QString &message)
{
    mMessage = message;
    emit updated(mMessage);
}
