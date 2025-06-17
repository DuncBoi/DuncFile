#ifndef XDB_H
#define XDB_H

#include <fstream>
#include <vector>
#include <system_error>
#include <cerrno>
#include <string>

namespace xdb {

template<typename T>
class XdbReader {
public:
    explicit XdbReader(const char* path)
      : ifs(path, std::ifstream::in | std::ifstream::binary)
    {
        if (ifs.fail()) {
            throw std::system_error(
                errno,
                std::generic_category(),
                std::string("std::ifstream"));
        }

        T rec;
        while (ifs >> rec) {
            list.push_back(rec);
        }
        if (ifs.bad()) {
            throw std::system_error(
                errno,
                std::generic_category(),
                std::string("std::ifstream::operator>>"));
        }
    }

    size_t size() const              { return list.size(); }
    const T& operator[](int i) const { return list[i]; }

protected:
    std::vector<T>  list;
    std::ifstream   ifs;
};

template<typename T>
class XdbWriter {
public:
    explicit XdbWriter(const char* path, bool append = false)
      : ofs(
            path,
            (append ? std::ofstream::app : std::ofstream::out)
            | std::ofstream::binary
        )
    {
        if (ofs.fail()) {
            throw std::system_error(
                errno,
                std::generic_category(),
                std::string("std::ofstream"));
        }
    }

    XdbWriter& operator<<(const T& rec) {
        if ((ofs << rec).bad()) {
            throw std::system_error(
                errno,
                std::generic_category(),
                std::string("std::ofstream::operator<<"));
        }
        return *this;
    }

private:
    std::ofstream ofs;
};

template<typename T>
class Xdb : public XdbWriter<T>, public XdbReader<T> {
public:
    explicit Xdb(const char* path)
      : XdbWriter<T>(path, true)
      , XdbReader<T>(path)
    {}

    Xdb& operator<<(const T& rec) {
        static_cast<XdbWriter<T>&>(*this) << rec;
        this->list.push_back(rec);
        return *this;
    }
};

}

#endif // XDB_H
