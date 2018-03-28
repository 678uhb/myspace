
#pragma once

#include "myspace/myspace_include.h"

myspace_begin

#ifdef myspace_linux

class Iconv
{
public:
    Iconv(const char* to, const char* from)
    :_iconv(iconv_open(to, from))
    {

    }

    ~Iconv()
    {
        iconv_close(_iconv);
    }

    string convert(const string& src)
    {
        if(src.empty()) return "";
        
        char* _inbuf = (char*)src.c_str();
		char** inbuf = &_inbuf;
        size_t _inbytesleft = sizeof(src.size());
        const size_t inbyteslength = _inbytesleft;
        size_t* inbytesleft = &_inbytesleft;

        auto dst = new_unique<char[]>(_inbytesleft);
		char* _oubuf = dst.get();
        char** oubuf = &_oubuf;
        size_t _outbytesleft = _inbytesleft;
        const size_t outbyteslength = _inbytesleft;
        size_t* outbytesleft = &_outbytesleft;

        string result;

        for(;;)
        {
            auto n = iconv(_iconv, inbuf, inbytesleft, oubuf, outbytesleft);

            if(n < 0)
            {
                if ( errno == EILSEQ ||  errno == EINVAL)
                {
                    *inbuf++;
                    *inbytesleft--;
                    continue;
                }
                else if( errno == E2BIG)
                {
                    result.append(dst.get(), outbyteslength - _outbytesleft);
                    _outbytesleft = outbyteslength;
                    _oubuf = dst.get();
                    continue;
                }
                else
                {
                    break;
                }
            }
            else
            {
                result.append(dst.get(), outbyteslength - _outbytesleft);
                break;
            }
        }

        return move(result);
    }
private:
    iconv_t     _iconv;
};

#endif

myspace_end

