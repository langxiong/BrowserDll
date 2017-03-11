#include "stdafx.h"
#include "MyPipe.h"
#include "Poco/Pipe.h"
using namespace Poco;
namespace MyWeb {

    MyPipe::MyPipe()
    {
    }


    MyPipe::~MyPipe()
    {
    }

    std::string MyPipe::ReadStr(const std::string & keyName)
    {
        return std::string();
    }

    bool MyPipe::WriteStr(const std::string & keyName, const std::string & value)
    {
        return false;
    }

    std::shared_ptr<Poco::Pipe> MyPipe::GetReaderPipe(int nID)
    {
        return m_pipeReaders[nID];
    }

    std::shared_ptr<Poco::Pipe> MyPipe::GetWriterPipe(int nID)
    {
        return m_pipeReaders[nID];
    }
} // MyWeb