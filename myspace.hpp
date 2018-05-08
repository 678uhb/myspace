
/* https://github.com/678uhb/myspace */

#pragma once

#include "myspace/algorithm/algorithm.hpp"
#include "myspace/any/any.hpp"
#include "myspace/codec/codec.hpp"
#include "myspace/concurrency/factory.hpp"
#include "myspace/config/config.hpp"
#include "myspace/coroutine/coroutine.hpp"
#include "myspace/defer/defer.hpp"
#include "myspace/detector/detector.hpp"
#include "myspace/dns/query.hpp"
#include "myspace/error/error.hpp"
#include "myspace/exception/exception.hpp"
#include "myspace/http/http.hpp"
#include "myspace/json/json.hpp"
#include "myspace/logger/logger.hpp"
#include "myspace/net/addr.hpp"
#include "myspace/net/netstream.hpp"
#include "myspace/net/socketopt.hpp"
#include "myspace/net/tcp/accepter.hpp"
#include "myspace/net/tcp/socket.hpp"
#include "myspace/net/udp/socket.hpp"
#include "myspace/os/os.hpp"
#include "myspace/pool/pool.hpp"
#include "myspace/process/process.hpp"
#include "myspace/strings/sstream.hpp"
#include "myspace/strings/strings.hpp"
#include "myspace/threadpool/threadpool.hpp"
#include "myspace/uuid/uuid.hpp"
#include "myspace/wav/file.hpp"
#include "myspace/wav/tool.hpp"
#include "myspace/wav/wav.hpp"

#if defined(MYSPACE_WINDOWS)
#include "myspace/microphone/microphone.hpp"
#endif
