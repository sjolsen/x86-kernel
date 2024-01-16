#ifndef X86_KERNEL_FORMAT_HH
#define X86_KERNEL_FORMAT_HH

#include <iostream>


template <typename... T>
std::string strcat(const T&... args) {
    std::stringstream ss;
    (ss << ... << args);
    return std::move(ss).str();
}


#endif  // X86_KERNEL_FORMAT_HH
