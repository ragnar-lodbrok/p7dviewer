////////////////////////////////////////////////////////////////////////////////
//                                                                             /
// 2012-2020 (c) Baical                                                        /
// 2022 (c) Ragnar Lodbrok                                                     /
//                                                                             /
// This library is free software; you can redistribute it and/or               /
// modify it under the terms of the GNU Lesser General Public                  /
// License as published by the Free Software Foundation; either                /
// version 3.0 of the License, or (at your option) any later version.          /
//                                                                             /
// This library is distributed in the hope that it will be useful,             /
// but WITHOUT ANY WARRANTY; without even the implied warranty of              /
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU           /
// Lesser General Public License for more details.                             /
//                                                                             /
// You should have received a copy of the GNU Lesser General Public            /
// License along with this library.                                            /
//                                                                             /
////////////////////////////////////////////////////////////////////////////////

#ifndef P7_DUMP_IMPORTER_H
#define P7_DUMP_IMPORTER_H

#include <QDebug>
#include <QDateTime>
#include <iostream>
#include <string>
#include <map>
#include "Formatter.h"
#include "p7Structs.h"

namespace p7 {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// <summary> Trace item description structure </summary>
struct stTrace
{
    /// <summary> Trace level </summary>
    eTraceLevel eLevel;
    /// <summary> Trace item unique Id </summary>
    uint16_t wID;
    /// <summary> Sequence number </summary>
    uint32_t dwSequence;
    /// <summary> Processor core number </summary>
    uint8_t bProcessor; //processor number
    /// <summary> Module Id </summary>
    uint32_t dwModuleID;
    /// <summary> Thread Id </summary>
    uint32_t dwThreadID;
    /// <summary>
    /// This time is offset from stream time: Bk::stStorageInfo::sStream_Time.
    /// To get absolute trace time (value representing the number of
    /// 100-nanosecond intervals since January 1, 1601 (UTC)):
    /// Bk::stStorageInfo::sStream_Time + stTrace::qwTime.
    ///</summary>
    uint64_t qwTime;
    /// <summary> Source file line number </summary>
    uint16_t wLine;
    /// <summary> File path </summary>
    QString pFile_Path;
    /// <summary> File name </summary>
    QString pFile_Name;
    /// <summary> Function name </summary>
    QString pFunction;
    /// <summary> Trace text </summary>
    QString pMessage;
    /// <summary> Trace text max size </summary>
    size_t szMessage;
    /// <summary> thread name </summary>
    QString pThreadName;
    /// <summary> module name </summary>
    QString pModuleName;
};


static QString traceLevelAsString(const eP7Trace_Level level)
{
    switch (level) {
    case EP7TRACE_LEVEL_TRACE:
        return "Trace";
    case EP7TRACE_LEVEL_DEBUG:
        return "Debug";
    case EP7TRACE_LEVEL_INFO:
        return "Info";
    case EP7TRACE_LEVEL_WARNING:
        return "Warning";
    case EP7TRACE_LEVEL_ERROR:
        return "Error";
    case EP7TRACE_LEVEL_CRITICAL:
        return "Critical";
    default:
        return QString();
    }
}

static QDateTime unpackDateTime(uint64_t datetime)
{
    if (datetime == 0) {
        return QDateTime();
    }

    const uint64_t TIME_MLSC_100NS = 10000ull;
    const uint64_t TIME_SEC_100NS = 10000000ull;
    const uint64_t TIME_OFFSET_1601_1970 = 116444736000000000ULL;

    uint32_t l_dwReminder = datetime % TIME_MLSC_100NS;

    datetime -= l_dwReminder;

    uint32_t l_dwMilli = (datetime % TIME_SEC_100NS) / TIME_MLSC_100NS;

    datetime -= TIME_OFFSET_1601_1970;

    time_t  l_llTime = datetime / TIME_SEC_100NS;
    tm     *l_pTime  = localtime(&l_llTime);

    if (l_pTime) {
        uint32_t rYear         = 1900 + l_pTime->tm_year;
        uint32_t rMonth        = 1 + l_pTime->tm_mon;
        uint32_t rDay          = l_pTime->tm_mday;
        uint32_t rHour         = l_pTime->tm_hour;
        uint32_t rMinutes      = l_pTime->tm_min;
        uint32_t rSeconds      = l_pTime->tm_sec;
        uint32_t rMilliseconds = l_dwMilli;

        QDate date = QDate(rYear, rMonth, rDay);
        QTime time = QTime(rHour, rMinutes, rSeconds, rMilliseconds);

        return QDateTime(date, time);
    }

    return QDateTime();
}

struct p7ThreadInfo
{
    uint32_t id = 0;
    QString name;
};

struct p7ModuleInfo
{
    uint16_t id = 0;
    eP7Trace_Level verbosity = EP7TRACE_LEVEL_COUNT;
    QString name;
};

struct p7DescriptionInfo
{
    p7DescriptionInfo() {}
    p7DescriptionInfo(const p7DescriptionInfo &) = delete;
    p7DescriptionInfo& operator=(const p7DescriptionInfo &) = delete;

    uint16_t id = 0;
    uint16_t line = 0;
    uint16_t moduleId = 0;
    uint16_t argsLen = 0;
    QString filename;
    QString function;

    sP7Trace_Arg *m_pArgs = nullptr; // pointer inside buffer

    std::vector<unsigned char> buffer;
    uint32_t bufferSize = 0;

    std::vector<char> format;
};

struct p7TraceDataInfo // ~= struct stTrace
{
    eP7Trace_Level verbosity = EP7TRACE_LEVEL_COUNT;
    uint16_t id = 0;
    uint32_t sequence = 0;
    uint8_t processorNumber = 0;
    uint32_t moduleId = 0;
    uint32_t threadId = 0;
    QDateTime time;
    uint16_t line = 0;
    QString filename;
    QString function;
    QString message;
    QString threadName;
    QString moduleName;
};

class p7DumpData
{
public:

    p7DumpData()
    {
        memset(&_header, 0, sizeof(_header));
    }

    sP7File_Header & header()
    {
        return _header;
    }

    uint64_t timerValue() const {
        return _qwTimer_Value;
    }

    void setTimerValue(uint64_t timerValue) {
        _qwTimer_Value = timerValue;
    }

    uint64_t timerFrequency() const {
        return _qwTimer_Frequency;
    }

    void setTimerFrequency(uint64_t timerFrequency) {
        _qwTimer_Frequency = timerFrequency;
    }

    QString hostName() const
    {
        return QString::fromUtf16((const char16_t *)_header.pHost_Name);
    }

    QString processName() const
    {
        return QString::fromUtf16((const char16_t *)_header.pProcess_Name);
    }

    QDateTime processDateTime() const
    {
        uint64_t l_qwTime
                = ((uint64_t)(_header.dwProcess_Start_Time_Hi) << 32)
                + (uint64_t)_header.dwProcess_Start_Time_Lo;

        return unpackDateTime(l_qwTime);
    }

    uint64_t processStartTime100Ns() const
    {
        return ((uint64_t)(_header.dwProcess_Start_Time_Hi) << 32)
                + (uint64_t)_header.dwProcess_Start_Time_Lo;
    }

    void addNewThread(const p7ThreadInfo & thread)
    {
        _threads[thread.id] = thread;
    }

    inline const p7ThreadInfo & threadById(uint32_t id) const
    {
        // TODO: seems we can have multiple threads with the same id when thread
        // stopped

        auto it = _threads.find(id);
        if (it != _threads.end()) {
            return it->second;
        } else {
            return _unknownThread;
        }
    }

    void addNewModule(const p7ModuleInfo & module)
    {
        _modules[module.id] = module;
    }

    inline const p7ModuleInfo & moduleById(uint16_t id) const
    {
        auto it = _modules.find(id);
        if (it != _modules.end()) {
            return it->second;
        } else {
            return _unknownModule;
        }
    }

    void addNewDescription(p7DescriptionInfo * desc)
    {
        _descriptions[desc->id] = std::shared_ptr<p7DescriptionInfo>(desc);
    }

    inline p7DescriptionInfo * descriptionById(uint16_t id) const
    {
        auto it = _descriptions.find(id);
        if (it != _descriptions.end()) {
            return it->second.get();
        } else {
            return nullptr;
        }
    }

    void addNewTraceData(const p7TraceDataInfo & data)
    {
        _traceData.push_back(data);
    }

    size_t traceDataCount() const
    {
        return _traceData.size();
    }

    const p7TraceDataInfo & traceDataAt(size_t index) const
    {
        return _traceData[index];
    }

    void addNewFormatter(CFormatter * formatter, uint16_t id)
    {
        _formatters[id] = std::shared_ptr<CFormatter>(formatter);
    }

    CFormatter * formatterById(uint16_t id) const
    {
        auto it = _formatters.find(id);
        if (it != _formatters.end()) {
            return it->second.get();
        } else {
            return nullptr;
        }
    }

private:

    std::map<uint32_t, p7ThreadInfo> _threads;
    std::map<uint16_t, p7ModuleInfo> _modules;
    std::map<uint16_t, std::shared_ptr<p7DescriptionInfo>> _descriptions;
    std::map<uint16_t, std::shared_ptr<CFormatter>> _formatters;
    std::vector<p7TraceDataInfo> _traceData;

    p7ModuleInfo _unknownModule;
    p7ThreadInfo _unknownThread;

    sP7File_Header _header;

    //QString _streamName;
    //Hi resolution timer value, we get this value when we retrieve current time.
    //using difference between this value and timer value for every trace we can
    //calculate time of the trace event with hi resolution
    uint64_t       _qwTimer_Value = 0;
    //timer's count heartbeats in second
    uint64_t       _qwTimer_Frequency = 0;
};

class p7DumpImporter
{
public:

    ~p7DumpImporter()
    {
        clear();
    }

    p7DumpData import(const std::string & fileName)
    {

        clear();

        p7DumpData data;

        _file = fopen(fileName.c_str(), "rb");

        if (!_file) {
            std::cerr << "Failed to open file";
            return data;
        }

        int fileSize = this->fileSize(_file);
        _allDataBuffer.resize(fileSize);

        _szData_Size = fread(_allDataBuffer.data(),
                                       sizeof(uint8_t),
                                       fileSize, _file);

        return importBufferToData(data);
    }

    p7DumpData import(const QByteArray &fileContent)
    {
        clear();

        p7DumpData data;

        _allDataBuffer.resize(fileContent.size());
        memcpy(_allDataBuffer.data(), fileContent.data(), fileContent.size());
        _szData_Size = fileContent.size();

        return importBufferToData(data);

    }

private:

    p7DumpData importBufferToData(p7DumpData & data)
    {
        if (sizeof(sP7File_Header) >= _szData_Size) {
            std::cerr << "File size less than header size should be";
            return data;
        }

        memcpy(&data.header(), _allDataBuffer.data(), sizeof(data.header()));

        sP7File_Header & header = data.header();

        if (P7_DAMP_FILE_MARKER_V1 != header.qwMarker) {
            if (ntohqw(P7_DAMP_FILE_MARKER_V1) == header.qwMarker) {

                std::cerr << "Little endian is not supported yet";

            } else {
                std::cerr << "Header is corrupted";
                return data;
            }
        }

        _szData_Offs  = sizeof(sP7File_Header);

        readData(data);

        return data;
    }

    void clear()
    {
        if (_file) {
            fclose(_file);
            _file = nullptr;
        }

        _szData_Offs = 0;
        _qwFile_Offs = 0;
        _qwFile_Size = 0;
        _szData_Size = 0;
    }

    int fileSize(FILE * file)
    {

        int pos = fseek(file, 0L, SEEK_CUR);
        fseek(file, 0L, SEEK_END);
        int size = ftell(file);

        fseek(file, pos, SEEK_SET);

        return size;
    }

    void readData(p7DumpData & data)
    {

        while(_szData_Offs + sizeof(sH_User_Data) <= _allDataBuffer.size())
        {
            sH_User_Data *l_pHeader = (sH_User_Data *)(_allDataBuffer.data()
                                                           + _szData_Offs);

            if ((_szData_Offs + l_pHeader->dwSize) > _allDataBuffer.size()) {
                break;
            }

            QByteArray dataChunk(
                  (const char *)(_allDataBuffer.data()
                   + _szData_Offs
                   + sizeof(sH_User_Data)),
                   l_pHeader->dwSize - sizeof(sH_User_Data));

            processDataChunk(dataChunk, data);

            _szData_Offs += l_pHeader->dwSize;
        }
    }

    void processDataChunk(QByteArray dataChunk, p7DumpData & data)
    {
        //sP7Trace_Data * traceData = (sP7Trace_Data *)dataChunk.data();

        //qDebug() << "  wId" << traceData->wID;
        //qDebug() << "  bLevel" << traceData->bLevel;
        //qDebug() << "  bProcessor" << traceData->bProcessor;
        //qDebug() << "  dwThreadID" << traceData->dwThreadID;
        //qDebug() << "  dwSequence" << traceData->dwSequence;
        //qDebug() << "  timer" << traceData->qwTimer;

        eResult l_eReturn = eOk;
        sP7Ext_Header *l_pHeader = NULL;

        while (dataChunk.size() && (eOk == l_eReturn)) {

            l_pHeader = (sP7Ext_Header *)dataChunk.data();

            //qDebug() << "  ** " << l_pHeader->dwType
            //         << l_pHeader->dwSize
            //         << dataChunk.size();

            if (    (EP7USER_TYPE_TRACE == l_pHeader->dwType)
                 || (dataChunk.size() >= l_pHeader->dwSize)
                )
            {
                l_eReturn = processPacket(l_pHeader, data);
            }

            if (eErrorMissmatch != l_eReturn) {
                dataChunk = dataChunk.mid(l_pHeader->dwSize);
            }
        }
    }

    eResult processPacket(sP7Ext_Header * i_pPacket, p7DumpData & data)
    {
        eResult l_eReturn = eOk;

        //qDebug() << " === subtype: " << i_pPacket->dwSubType;

        if (EP7TRACE_TYPE_DATA == i_pPacket->dwSubType) {

            return processDataPacket(i_pPacket, data);

        } else if (EP7TRACE_TYPE_INFO == i_pPacket->dwSubType) {

            return processInfoPacket(i_pPacket, data);

        } else if (EP7TRACE_TYPE_THREAD_START == i_pPacket->dwSubType) {

            return processThreadStartPacket(i_pPacket, data);

        } else if (EP7TRACE_TYPE_MODULE == i_pPacket->dwSubType) {

            return processModulePacket(i_pPacket, data);

        } else if (EP7TRACE_TYPE_DESC == i_pPacket->dwSubType) {

            return processDescPacket(i_pPacket, data);

        } else if (EP7TRACE_TYPE_CLOSE == i_pPacket->dwSubType) {

            return eErrorClosed;
        }

        return l_eReturn;
    }

    eResult processDataPacket(sP7Ext_Header * i_pPacket, p7DumpData & data)
    {
        //qDebug() << "  -- EP7TRACE_TYPE_DATA";

        sP7Trace_Data *l_pTrace = (sP7Trace_Data*)i_pPacket;

        p7TraceDataInfo traceData;
        traceData.id = l_pTrace->wID;
        traceData.verbosity = (eP7Trace_Level)l_pTrace->bLevel;
        traceData.processorNumber = l_pTrace->bProcessor;
        traceData.sequence = l_pTrace->dwSequence;
        traceData.threadId = l_pTrace->dwThreadID;

        p7DescriptionInfo * desc = data.descriptionById(traceData.id);
        if (desc) {
            traceData.filename = desc->filename;
            traceData.function = desc->function;
            traceData.moduleId = desc->moduleId;
            traceData.line     = desc->line;
        } else {
            traceData.filename
                = QString("Unable to find description %1").arg(traceData.id);
        }

        const p7ModuleInfo & moduleInfo = data.moduleById(traceData.moduleId);
        traceData.moduleName = moduleInfo.name;

        const p7ThreadInfo & threadInfo = data.threadById(traceData.threadId);
        traceData.threadName = threadInfo.name;

        uint64_t timeOffset
            = (uint64_t)(
                    (double)(l_pTrace->qwTimer - data.timerValue()) * 10000000.0
                    / (double)data.timerFrequency());

        traceData.time = unpackDateTime(
                    data.processStartTime100Ns() + timeOffset);

        const size_t traceMessageBufSize = (0x2000);
        tXCHAR traceMessageBuf[traceMessageBufSize];
        memset(traceMessageBuf, 0, traceMessageBufSize);

        CFormatter * formatter = data.formatterById(desc->id);
        if (formatter) {

            int32_t formatRes = formatter->Format(
                        traceMessageBuf,
                        traceMessageBufSize,
                        (const unsigned char *)((tUINT8*)i_pPacket + sizeof(sP7Trace_Data)));

            if (0 < formatRes)
            {
                tXCHAR *l_pIter = traceMessageBuf;
                while (*l_pIter)
                {
                    if (    (10 == *l_pIter)
                         || (13 == *l_pIter)
                       )
                    {
                        *l_pIter = ';';
                    }

                    ++l_pIter;
                }

                traceData.message = QString::fromUtf8(traceMessageBuf);
            } else {
                traceData.message = "Unable to format the message";
            }
        } else {
            traceData.message = "No formatter found";
        }

        /*qDebug() << "TRACE: ******\n"
                 << "id:" << traceData.id << "\n"
                 << "level:" << traceLevelAsString(traceData.verbosity) << "\n"
                 << "module" << traceData.moduleName
                 << "(" << traceData.moduleId << ")\n"
                 << "CPU #" << traceData.processorNumber << "\n"
                 << "thread:" << traceData.threadName
                 << "(" << QString::number(traceData.threadId, 16) << ")\n"
                 << "file:" << traceData.filepath << "\\"
                 << traceData.filename << "\n"
                 << "line:" << traceData.line << "\n"
                 << "function:" << traceData.function << "\n"
                 << "time:" << traceData.time << "\n"
                 << "text:" << traceData.message << "\n";*/


        data.addNewTraceData(std::move(traceData));

        return eOk;
    }

    eResult processInfoPacket(sP7Ext_Header * i_pPacket, p7DumpData & data)
    {
        //qDebug() << "  -- EP7TRACE_TYPE_INFO";

        sP7Trace_Info *l_pInfo = (sP7Trace_Info *)i_pPacket;

        // TODO:
        //QString streamName = QString::fromUtf16(
        //            (const char16_t *)l_pInfo->pName);

        data.setTimerValue(l_pInfo->qwTimer_Value);
        data.setTimerFrequency(l_pInfo->qwTimer_Frequency);

        return eOk;
    }

    eResult processThreadStartPacket(sP7Ext_Header * i_pPacket,
                                     p7DumpData &data)
    {
        //qDebug() << "  -- EP7TRACE_TYPE_THREAD_START";

        sP7Trace_Thread_Start *l_pThStart = (sP7Trace_Thread_Start*)i_pPacket;

        uint32_t treadId = l_pThStart->dwThreadID;
        QString threadName = QString::fromUtf8(
                    (const char *)l_pThStart->pName);

        //qDebug() << "  -- " << treadId << threadName;
        data.addNewThread({treadId, threadName});

        return eOk;
    }

    eResult processModulePacket(sP7Ext_Header * i_pPacket,
                                p7DumpData &data)
    {
        //qDebug() << "  -- EP7TRACE_TYPE_MODULE";

        sP7Trace_Module *l_pModule = (sP7Trace_Module*)i_pPacket;

        uint16_t moduleId = l_pModule->wModuleID;
        QString moduleName = QString::fromUtf8(
                    (const char *)l_pModule->pName);
        eP7Trace_Level verbosity = l_pModule->eVerbosity;

        //qDebug() << "  -- " << moduleId << moduleName << verbosity;
        data.addNewModule({moduleId, verbosity, moduleName});

        return eOk;
    }

    eResult processDescPacket(sP7Ext_Header * i_pPacket,
                              p7DumpData &data)
    {
        //qDebug() << "  -- EP7TRACE_TYPE_DESC";

        sP7Trace_Format *l_pDesc = (sP7Trace_Format*)i_pPacket;

        p7DescriptionInfo * desc = new p7DescriptionInfo();

        desc->id = l_pDesc->wID;
        desc->line = l_pDesc->wLine;
        desc->moduleId = l_pDesc->wModuleID;
        desc->argsLen = l_pDesc->wArgs_Len;

        desc->bufferSize = l_pDesc->sCommon.dwSize;
        desc->buffer.resize(desc->bufferSize);

        tWCHAR *l_pFormat = nullptr;

        if (desc->buffer.size()) {
            memcpy(desc->buffer.data(), l_pDesc, desc->bufferSize);
            desc->m_pArgs
                    = (sP7Trace_Arg*)(desc->buffer.data() + sizeof(sP7Trace_Format));

            l_pFormat = (tWCHAR *)(desc->m_pArgs + desc->argsLen);
            size_t l_szLen  = Get_UTF16_Length(l_pFormat) + 1;
            desc->format.resize(l_szLen * 2);
            Convert_UTF16_To_UTF8(l_pFormat, desc->format.data(), l_szLen * 2);

            tXCHAR * m_pFile_Path = (char*)(l_pFormat + l_szLen);
            tXCHAR * m_pFile_Name = nullptr;

            if ((m_pFile_Name = strrchr(m_pFile_Path, '/'))) {
                m_pFile_Name ++;
            } else {
                m_pFile_Name = m_pFile_Path;
            };

            char * pFunction = (char*)(m_pFile_Path + strlen(m_pFile_Path) + 1);
            desc->function = QString::fromUtf8(pFunction);
            desc->filename = QString::fromUtf8(m_pFile_Name);
        }

        //qDebug() << "  -- " << desc->id
        //         << data.moduleById(desc->moduleId).name
        //         << desc->line
        //         << desc->argsLen
        //         << desc->filename
        //         << desc->function;

        data.addNewDescription(desc);

        if (desc->format.size()) {
            CFormatter * formatter = new CFormatter(
                    (const char *)desc->format.data(),
                    desc->m_pArgs,
                    (size_t)desc->argsLen,
                    new CFormatter::sBuffer(8192));

            data.addNewFormatter(formatter, desc->id);
        }

        return eOk;
    }

    FILE * _file = nullptr;
    size_t _szData_Offs = 0;
    size_t _szData_Size = 0;
    uint64_t _qwFile_Offs = 0;
    uint64_t _qwFile_Size = 0;

    std::vector<uint8_t> _allDataBuffer;

};

}



#endif // P7_DUMP_IMPORTER_H
