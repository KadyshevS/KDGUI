#include <precomp.h>

#include <Debug/KDException.h>

namespace KDE
{
    KDException::KDException(int line, const std::string &file)
        : m_Line(line), m_File(file)
    {
    }
    KDException::~KDException()
    {
    }

    const char* KDException::what() const
    {
        std::stringstream ss;
        ss  << type() << "\n\n"
            << "[Line]: " << m_Line << '\n'
            << "[File]: " << m_File;
        
        m_WhatBuffer = ss.str();
        return m_WhatBuffer.c_str();
    }
    const char* KDException::type() const
    {
        return "Standard KDException";
    }
}
