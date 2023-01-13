//
// Created by YiMing D on 2023/1/13.
//

#ifndef DREAMER_NOCOPYABLE_H
#define DREAMER_NOCOPYABLE_H


class NoCopyable {
public:
    NoCopyable() = default;
    ~NoCopyable() = default;
    NoCopyable(const NoCopyable&) = delete;
    NoCopyable& operator=(const NoCopyable&) = delete;
};

#endif //DREAMER_NOCOPYABLE_H
