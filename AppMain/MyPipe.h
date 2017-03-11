#pragma once

#include "Poco/Pipe.h"
#include <memory>
#include <map>

namespace MyWeb {
    class MyPipe
    {
    public:
        MyPipe();
        ~MyPipe();

        std::string ReadStr(const std::string& keyName);
        bool WriteStr(const std::string& keyName, const std::string& value);

        std::shared_ptr<Poco::Pipe> GetReaderPipe(int nID);
        std::shared_ptr<Poco::Pipe> GetWriterPipe(int nID);
    private:
        std::map<int, std::shared_ptr<Poco::Pipe>> m_pipeReaders;
        std::map<int, std::shared_ptr<Poco::Pipe>> m_pipeWriters;
    };
} // MyWeb


