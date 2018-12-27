#ifndef SECTION
#define SECTION

#include <QVector>

/*
 * Utility class to access memory areas instead just as bare pointers.
 * Future: non-const and std::iterators
 * */

template <typename T> class Section{
public:
    Section() : m_ptr(nullptr), m_len(0){}
    Section(const T* data, int len) : m_ptr(data), m_len(len){}
    Section(const QVector<T>& data): m_ptr(data.data()), m_len(data.length()){}
    Section(const QVector<T>& data, int start, int len) : m_ptr(data.data() + start), m_len(len){}
    Section(const Section& section) : m_ptr(section.data()), m_len(section.length()){}
    Section(const Section& section, int start, int len) : m_ptr(section.data() + start), m_len(len){}
    operator QVector<T> ()  {QVector<T> vec(m_len ,0); memcpy(vec.data(), m_ptr, m_len * sizeof(T)); return vec;}
    const T* data() const {return m_ptr;}
    T* data() { return const_cast<T*>(m_ptr);}
    int length() const {return m_len;}
    const T* begin() {return m_ptr;}
    const T* end() {return m_ptr + m_len;}
private:
    const T* m_ptr;
    const int m_len;
};

#endif // SECTION

