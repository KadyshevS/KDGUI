#pragma once

#include <Debug/KDException.h>

#define NOMINMAX
#include <Windows.h>

#include <sstream>

class WndException : public KDE::KDException
{
    public:
        using KDE::KDException::KDException;

        static std::string TranslateErrorCode(HRESULT hr)
        {
            char* pMsgBuf = nullptr;
            const DWORD nMsgLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,hr,MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),
                reinterpret_cast<LPSTR>(&pMsgBuf),0,nullptr
            );
            if( nMsgLen == 0 )
            {
                return "Unidentified error code";
            }
            std::string errorString = pMsgBuf;
            LocalFree( pMsgBuf );
            return errorString;
        }
};
class WndHrException : public WndException
{
    public:
        WndHrException(int line, const char* file, HRESULT hr)
            : WndException(line, file), m_Hr(hr)
        {
        }

        virtual const char* type() const override
        {
            return "KDWindow Exception";
        }
        virtual const char* what() const noexcept override
        {
            std::stringstream ss;
            ss  << type() << "\n\n"
            << "[Line]: " << m_Line << '\n'
            << "[File]: " << m_File << '\n'
            << "[Description]: " << TranslateErrorCode(m_Hr);
        
            m_WhatBuffer = ss.str();
            return m_WhatBuffer.c_str();
        }

        static std::string TranslateErrorCode(HRESULT hr)
        {
            char* pMsgBuf = nullptr;
            const DWORD nMsgLen = FormatMessage(
                FORMAT_MESSAGE_ALLOCATE_BUFFER |
                FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                nullptr,hr,MAKELANGID( LANG_NEUTRAL,SUBLANG_DEFAULT ),
                (LPSTR)&pMsgBuf,0,nullptr
            );
            if( nMsgLen == 0 )
            {
                return "Unidentified error code";
            }
            std::string errorString = pMsgBuf;
            LocalFree( pMsgBuf );
            return errorString;
        }

    private:
        HRESULT m_Hr;
};

#ifndef NDEBUG
    #define KDWND_EXCEPT(hr) WndHrException(__LINE__, __FILE__, hr)
    #define KDWND_LAST_EXCEPT WndHrException(__LINE__, __FILE__, GetLastError())
    #define KDWND_THROW_FAILED(hrcall) if( FAILED(hr = (hrcall)) ) throw KDWND_EXCEPT(hr)
    #define KDWND_THROW_LAST_EXCEPT(hrcall) if( FAILED(hr = (hrcall)) ) throw KDWND_LAST_EXCEPT
#else
    #define KDWND_EXCEPT(hr)
    #define KDWND_LAST_EXCEPT
    #define KDWND_THROW_FAILED(hrcall) (hrcall);
    #define KDWND_THROW_LAST_EXCEPT(hrcall) (hrcall);
#endif