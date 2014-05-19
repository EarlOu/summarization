#ifndef UTIL
#define UTIL

#include <opencv2/opencv.hpp>

#define CV_INIT_ALGORITHM(classname, algname, memberinit) \
    static ::cv::Algorithm* create##classname() \
    { \
        return new classname; \
    } \
    \
    static ::cv::AlgorithmInfo& classname##_info() \
    { \
        static ::cv::AlgorithmInfo classname##_info_var(algname, create##classname); \
        return classname##_info_var; \
    } \
    \
    static ::cv::AlgorithmInfo& classname##_info_auto = classname##_info(); \
    \
    ::cv::AlgorithmInfo* classname::info() const \
    { \
        static volatile bool initialized = false; \
        \
        if( !initialized ) \
        { \
            initialized = true; \
            classname obj; \
            memberinit; \
        } \
        return &classname##_info(); \
    }

#endif
