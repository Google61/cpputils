#pragma once
#include <memory>
#include <vector>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <io.h>
#endif
#ifndef _WIN32
#include <sys/stat.h>

#ifdef __MACH__
#include <sys/disk.h>
#endif
#ifdef __linux__
#include <linux/fs.h>
#endif

#include <sys/ioctl.h>
#include <unistd.h>
#endif
#include <fcntl.h>

// wrap posix file handle in a c++ class,
// so it will be closed when it leaves the current scope.
// This is most useful for making code exception safe.


struct filehandle {

    struct ptr {
        int fh = -1;
        ptr(int fh)
            : fh(fh)
        {
        }
        ~ptr() { close(); }

        void close()
        {
            if (fh==-1)
                return;
            if (-1 == ::close(fh)) {
                fh = -1;
                throw std::system_error(errno, std::generic_category(), "close");
            }
            fh = -1;
        }
    };
    std::shared_ptr<ptr> _p;


    filehandle() { }

    filehandle(const filehandle& fh)
    {
        *this = fh;
    }

    filehandle(int fh)
    {
        *this = fh;
    }
    filehandle(const std::string& filename, int openflags = O_RDONLY, int mode=0666)
    {
        *this = open(filename.c_str(), openflags, mode);
    }

    filehandle& operator=(int fh)
    { 
        if (fh==-1)
            throw std::system_error(errno, std::generic_category());
        _p = std::make_shared<ptr>(fh);
        return *this;
    }
    filehandle& operator=(const filehandle& fh)
    {
        _p = fh._p;
        return *this;
    }
    bool empty() const { return !_p; }

    operator bool () const { return !empty(); }
    operator int () const { return fh(); }
    int fh() const {
        if (!_p) throw std::runtime_error("no filehandle set");
        return _p->fh;
    }
    void close() {
        if (!_p) throw std::runtime_error("no filehandle set");
        _p->close();
    }

    int64_t size()
    {
        struct stat st;
        if (fstat(fh(), &st))
            return -1;
        if (st.st_mode&S_IFREG)
            return st.st_size;
        else if (st.st_mode&S_IFBLK) {
#ifdef DKIOCGETBLOCKCOUNT
            uint64_t bkcount;
            uint32_t bksize;
            if (-1==ioctl(fh(), DKIOCGETBLOCKCOUNT, &bkcount))
                return -1;
            if (-1==ioctl(fh(), DKIOCGETBLOCKSIZE, &bksize))
                return -1;
            return bkcount*bksize;
#endif
#ifdef BLKGETSIZE64
            uint64_t devsize;
            if (-1==ioctl(fh(), BLKGETSIZE64, &devsize))
                return -1;
            return devsize;
#endif
        }
        else {
            // pipe or socket
            return -1;
        }
    }

    uint64_t seek(int64_t pos, int whence=SEEK_SET)
    {
        auto rc = ::lseek(fh(), pos, whence);
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "lseek");
        return rc;
    }
    uint64_t tell()
    {
        return seek(0, SEEK_CUR);
    }
    void trunc(uint64_t pos)
    {
        if (-1==::ftruncate(fh(), pos))
            throw std::system_error(errno, std::generic_category(), "truncate");
    }
    template<typename RANGE>
    auto write(const RANGE& r)
    {
        auto rc = ::write(fh(), &*r.begin(), r.size());
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "write");
        
        return rc;
    }
    template<typename PTR>
    auto write(PTR first, PTR last)
    {
        auto rc = ::write(fh(), first, std::distance(first, last)*sizeof(*first));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "write");
        
        return rc;
    }
    template<typename PTR>
    auto write(PTR ptr, size_t count)
    {
        auto rc = ::write(fh(), ptr, count*sizeof(*ptr));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "write");
        
        return rc;
    }
    template<typename INT>
    std::enable_if_t<std::is_scalar_v<INT>, std::vector<uint8_t>> read(INT count)
    {
        std::vector<uint8_t> data(count);
        auto rc = ::read(fh(), &data[0], data.size());
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "read");

        data.resize(rc);

        return data;
    }
    template<typename PTR>
    size_t read(PTR first, PTR last)
    {
        auto rc = ::read(fh(), first, std::distance(first, last)*sizeof(*first));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "read");

        return rc;
    }
    template<typename PTR>
    size_t read(PTR ptr, size_t count)
    {
        auto rc = ::read(fh(), ptr, count*sizeof(*ptr));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "read");

        return rc;
    }

    template<typename RANGE>
    std::enable_if_t<!std::is_scalar_v<RANGE>, size_t> read(RANGE r)
    {
        auto rc = ::read(fh(), &*r.begin(), r.size() * sizeof(RANGE::value_type));
        if (rc == -1)
            throw std::system_error(errno, std::generic_category(), "read");

        return rc;
    }
};
