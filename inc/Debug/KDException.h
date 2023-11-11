#pragma once

#include <string>
#include <exception>

namespace KDE
{
    class KDException : public std::exception
    {
        public:
            KDException(int line, const std::string& file);
            ~KDException();

            virtual const char* what() const noexcept override;
            virtual const char* type() const;

        protected:
            mutable std::string m_WhatBuffer;
            std::string m_File;
            int m_Line;
    };
}

#define KD_STD_EXCEPT KDE::KDException(__LINE__, __FILE__)