#ifndef _TLOGGER_H_
#define _TLOGGER_H_

#include <iostream> 
#include <ostream>
#include <sstream>
#include <string>

template<class Head>
void print_args_(std::ostream& s, Head&& head) {
    s << std::forward<Head>(head);
}

template<class Head, class... Tail>
void print_args_(std::ostream& s, Head&& head, Tail&&... tail) {
    s << std::forward<Head>(head) << " ";
    print_args_(s, std::forward<Tail>(tail)...);
}

template<class... Args>
void print_args(Args&&... args) {
    print_args_(std::cout, std::forward<Args>(args)...);
}

#endif