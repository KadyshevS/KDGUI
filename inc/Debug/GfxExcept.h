#pragma once

#define NOMINMAX
#include <Windows.h>

#include <Debug/KDException.h>
#include <Debug/dxerr.h>
#include <sstream>
#include <d3d11.h>
#include <vector>

class GfxException : public KDE::KDException
{
    public:
        using KDE::KDException::KDException;
};
class GfxHrException : public GfxException
{
    public:
        GfxHrException(int line, const char* file, HRESULT hr, std::vector<std::string> infoMsgs = {})
            : GfxException(line, file), m_Hr(hr)
        {
            for (auto& msg : infoMsgs)
            {
                m_Info += msg;
                m_Info += '\n';
            }
        }

        virtual const char* type() const override
        {
            return "KDGraphics Exception";
        }
        virtual const char* what() const override
        {
            std::stringstream ss;
            ss  << type() << "\n\n"
                << "[Error Code]: 0x" << std::hex << GetErrorCode()
                << std::dec << (unsigned long)GetErrorCode() << '\n'
                << "[Error String]: " << GetErrorString() << '\n'
                << "[Description]: " << GetErrorDescription() << '\n';
            if (!m_Info.empty())
            ss  << "[Error Info]: \n" << GetErrorInfo() << '\n';
            ss  << "[Line]: " << m_Line << '\n'
                << "[File]: " << m_File << '\n';
        
            m_WhatBuffer = ss.str();
            return m_WhatBuffer.c_str();
        }

        HRESULT GetErrorCode() const { return m_Hr; }
        std::string GetErrorString() const 
        {
            return DXGetErrorString(m_Hr);
        }
        std::string GetErrorDescription() const 
        {
            char buf[512];
            DXGetErrorDescription(m_Hr, buf, sizeof(buf));
            return buf;
        }
        std::string GetErrorInfo() const
        {
            return m_Info;
        }

    private:
        HRESULT m_Hr;
        std::string m_Info;
};
class GfxInfoException : public GfxException
{
    public:
        GfxInfoException(int line, const char* file, std::vector<std::string> infoMsgs)
            : GfxException(line, file)
        {
            for (auto& msg : infoMsgs)
            {
                m_Info += msg;
                m_Info += '\n';
            }
        }

        virtual const char* type() const override
        {
            return "KDGraphics Info Exception";
        }
        virtual const char* what() const override
        {
            std::stringstream ss;
            ss  << type() << "\n\n"
                << "[Error Info]: " << GetErrorInfo() << '\n'
                << "[Line]: " << m_Line << '\n'
                << "[File]: " << m_File << '\n';
        
            m_WhatBuffer = ss.str();
            return m_WhatBuffer.c_str();
        }

        std::string GetErrorInfo() const
        {
            return m_Info;
        }

    private:
        std::string m_Info;
};
class GfxDeviceRemovedException : public GfxHrException
{
    public:
        using GfxHrException::GfxHrException;

        virtual const char* type() const override
        {
            return "KDGraphics Device Removed Exception";
        }
};

#ifndef NDEBUG
    #define INFOMAN(renderer)               auto& infoManager = GetInfoManager(renderer); HRESULT hr

    #define KDGFX_EXCEPT(hr)                GfxHrException( __LINE__,__FILE__,(hr),infoManager.GetMessages() )

    #define KDGFX_THROW_INFO(hrcall)        infoManager.Set(); \
                                            if( FAILED(hr = (hrcall) ) ) \
                                                throw KDGFX_EXCEPT(hr)

    #define KDGFX_EXCEPT_NOINFO(hr)         GfxHrException(__LINE__, __FILE__, (hr))

    #define KDGFX_THROW_NOINFO(hrcall)      if( FAILED(hr = (hrcall)) ) \
                                                throw KDGFX_EXCEPT_NOINFO(hr)

    #define KDGFX_DEVICE_REMOVED_EXCEPT(hr) GfxDeviceRemovedException(__LINE__,__FILE__, (hr), infoManager.GetMessages())

    #define KDGFX_THROW_INFO_ONLY(call)     infoManager.Set(); \
                                            (call); \
                                            { \
                                               auto v = infoManager.GetMessages(); \
                                               if(!v.empty()) \
                                                   throw GfxInfoException(__LINE__,__FILE__,v); \
                                            }
#else
    #define INFOMAN(renderer)
    #define KDGFX_EXCEPT(hr)
    #define KDGFX_THROW_INFO(hrcall) (hrcall);
    #define KDGFX_EXCEPT_NOINFO(hr)
    #define KDGFX_THROW_NOINFO(hrcall) (hrcall);
    #define KDGFX_DEVICE_REMOVED_EXCEPT(hr)
    #define KDGFX_THROW_INFO_ONLY(call) (call);
#endif