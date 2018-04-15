
#pragma once

#include "myspace/_/stdafx.hpp"
#include "myspace/strings/strings.hpp"

#if defined(MYSPACE_WINDOWS)
#include "myspace/dns/_/dnsquery.hpp"
#endif

MYSPACE_BEGIN

namespace dns {

inline std::string query(const std::string& domain_name);

#if defined(MYSPACE_WINDOWS)
inline std::string query(const std::string& domain_name){
    if(domain_name.size() >= 4) {
        if(Strings::tolower(domain_name.substr(0,4)) == "www."){
            return dns::dnsQuery(domain_name.substr(4)).a_;
        }
    }
    return dns::dnsQuery(domain_name).a_;
}
#endif

#if defined(MYSPACE_LINUX)
inline std::string query(const std::string& domain_name) {
    return "";
}
#endif
} // namespace dns

MYSPACE_END
