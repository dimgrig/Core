#ifndef BIT_H
#define BIT_H

//same type of arguments
template <class T>
static inline void set_bit(T &v, T bit) {
    v |= ((T)1 << bit);
}

template <class T>
static inline void reset_bit(T &v, T bit) {
    v &= ~((T)1 << bit);
}

template <class T>
static inline T toggle_bit(T &v, T bit) {
    v ^= ((T)1 << bit);
}

template <class T>
static inline T set_bit_ret(T v, T bit) {
    return (v | ((T)1 << bit));
}

template <class T>
static inline T reset_bit_ret(T v, T bit) {
    return (v & (~((T)1 << bit)));
}

template <class T>
static inline T toggle_bit_ret(T v, T bit) {
    return (v ^ ((T)1 << bit));
}

template <class T>
static inline bool bit_check(T v, T bit) {
    return ((v >> bit) & (T)1);
}



//bit argument casts to value
template <class T, class ST>
static inline void set_bit(T &v, ST bit) {
    v |= ((T)1 << (T)bit);
}

template <class T, class ST>
static inline void reset_bit(T &v, ST bit) {
    v &= ~((T)1 << (T)bit);
}

template <class T, class ST>
static inline T toggle_bit(T &v, ST bit) {
    v ^= ((T)1 << (T)bit);
}

template <class T, class ST>
static inline T set_bit_ret(T v, ST bit) {
    return (v | ((T)1 << (T)bit));
}

template <class T, class ST>
static inline T reset_bit_ret(T v, ST bit) {
    return (v & (~((T)1 << (T)bit)));
}

template <class T, class ST>
static inline T toggle_bit_ret(T v, ST bit) {
    return (v ^ ((T)1 << (T)bit));
}

template <class T, class ST>
static inline bool bit_check(T v, ST bit) {
    return ((v >> (T)bit) & (T)1);
}
#endif // BIT_H
