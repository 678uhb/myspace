
#include "myspace/myspace_include.h"
#include "myspace/wave/wave.hpp"

myspace_begin

class WaveTool
{
public:
    static uint8_t combine(uint8_t pcm1, uint8_t pcm2)
    {
        uint16_t combine = pcm1 + pcm2;

        if(combine > UINT8_MAX) return UINT8_MAX;

        return (uint8_t)combine;
    }

    static int16_t combine(int16_t pcm1 , int16_t pcm2)
    {
        int32_t combined = pcm1 + pcm2;

        if(combined > INT16_MAX) return INT16_MAX;

        if(combined < INT16_MIN) return INT16_MIN;

        return (int16_t)combined;
    }   

    template<class X>
    static X combine(const X& pcm1, const X& pcm2)
    {
        X result;

        auto itr1 = begin(pcm1);

        auto itr2 = begin(pcm2);

        while(itr1 != end(pcm1) || itr2 != end(pcm2))
        {
            auto first = (itr1 == end(pcm1) ? 0 : *itr1 );

            auto second = (itr2 == end(pcm2) ? 0 : *itr2 );

            result.push_back(combine(first, second));

            ++itr1;

            ++itr2;
        }

        return move(result);
    }

    static string combine_16_samplebits(const string& pcm1, const string& pcm2)
    {
        string result;

        for( size_t i = 0, j = 0; i + 1 < pcm1.size() || j + 1 < pcm2.size(); )
        {
            int16_t first = ( i < pcm1.size() ? *((int16_t*)&pcm1[i]) : 0 );

            int16_t second = ( j < pcm2.size() ? *((int16_t*)&pcm2[j]) : 0 );

            int16_t combined = combine(first, second);

            result.append((char*)&combined, 2);

            i += 2;

            j += 2;
        }

        return move(result);
    }

};

myspace_end
