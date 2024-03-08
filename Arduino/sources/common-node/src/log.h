#ifndef LOG_H
#define LOG_H

class Print;
extern Print &dbgOutput;

template <typename... Args> void log_impl(Args &&...args);

template <typename Value> void log_impl(Value &&v)
{
    dbgOutput.print(v);
}

template <typename First, typename... Args> void log_impl(First &&v, Args &&...args)
{
    log_impl(v);
    log_impl(args...);
}

template <typename... Args> inline void log(Args &&...args)
{
    log_impl(args...);
    dbgOutput.println();
}

#ifdef DEBUG_MESSAGES
#define LOG_DEBUG(...) log("DBG\t", __VA_ARGS__)
#define LOG_INFO(...) log("INF\t", __VA_ARGS__)
#else
#define LOG_DEBUG(...)
#define LOG_INFO(...)
#endif

#define LOG_WARNING(...) log("WRN\t", __VA_ARGS__)
#define LOG_ERROR(...) log("ERR\t", __VA_ARGS__)

#endif