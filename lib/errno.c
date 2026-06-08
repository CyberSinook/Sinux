#include "errno.h"

int errno = 0;

const char *
strerror(int errnum)
{
    switch (errnum) {
        case 0:           return "Success";
        case EPERM:       return "Operation not permitted";
        case ENOENT:      return "No such file or directory";
        case ESRCH:       return "No such process";
        case EINTR:       return "Interrupted system call";
        case EIO:         return "I/O error";
        case ENXIO:       return "No such device or address";
        case E2BIG:       return "Argument list too long";
        case ENOEXEC:     return "Exec format error";
        case EBADF:       return "Bad file descriptor";
        case ECHILD:      return "No child processes";
        case EAGAIN:      return "Try again";
        case ENOMEM:      return "Out of memory";
        case EACCES:      return "Permission denied";
        case EFAULT:      return "Bad address";
        case ENOTBLK:     return "Block device required";
        case EBUSY:       return "Device or resource busy";
        case EEXIST:      return "File exists";
        case EXDEV:       return "Cross-device link";
        case ENODEV:      return "No such device";
        case ENOTDIR:     return "Not a directory";
        case EISDIR:      return "Is a directory";
        case EINVAL:      return "Invalid argument";
        case ENFILE:      return "File table overflow";
        case EMFILE:      return "Too many open files";
        case ENOTTY:      return "Not a typewriter";
        case ETXTBSY:     return "Text file busy";
        case EFBIG:       return "File too large";
        case ENOSPC:      return "No space left on device";
        case ESPIPE:      return "Illegal seek";
        case EROFS:       return "Read-only file system";
        case EMLINK:      return "Too many links";
        case EPIPE:       return "Broken pipe";
        case EDOM:        return "Math argument out of domain";
        case ERANGE:      return "Math result not representable";
        case EDEADLK:     return "Resource deadlock would occur";
        case ENAMETOOLONG: return "File name too long";
        case ENOLCK:      return "No record locks available";
        case ENOSYS:      return "Function not implemented";
        case ENOTEMPTY:   return "Directory not empty";
        case ELOOP:       return "Too many symbolic links encountered";
        case ENOMSG:      return "No message of desired type";
        case EIDRM:       return "Identifier removed";
        case ENODATA:     return "No data available";
        case ETIME:       return "Timer expired";
        case EPROTO:      return "Protocol error";
        case EOVERFLOW:   return "Value too large for defined data type";
        case EILSEQ:      return "Illegal byte sequence";
        case ENOTSOCK:    return "Socket operation on non-socket";
        case EMSGSIZE:    return "Message too long";
        case EPROTOTYPE:  return "Protocol wrong type for socket";
        case ENOPROTOOPT: return "Protocol not available";
        case EPROTONOSUPPORT: return "Protocol not supported";
        case EOPNOTSUPP:  return "Operation not supported on transport endpoint";
        case EAFNOSUPPORT: return "Address family not supported";
        case EADDRINUSE:  return "Address already in use";
        case EADDRNOTAVAIL: return "Cannot assign requested address";
        case ENETDOWN:    return "Network is down";
        case ENETUNREACH: return "Network is unreachable";
        case ENETRESET:   return "Network dropped connection because of reset";
        case ECONNABORTED: return "Software caused connection abort";
        case ECONNRESET:  return "Connection reset by peer";
        case ENOBUFS:     return "No buffer space available";
        case EISCONN:     return "Transport endpoint is already connected";
        case ENOTCONN:    return "Transport endpoint is not connected";
        case ETIMEDOUT:   return "Connection timed out";
        case ECONNREFUSED: return "Connection refused";
        case EHOSTDOWN:   return "Host is down";
        case EHOSTUNREACH: return "No route to host";
        case EALREADY:    return "Operation already in progress";
        case EINPROGRESS: return "Operation now in progress";
        case ESTALE:      return "Stale file handle";
        case EDQUOT:      return "Quota exceeded";
        case ECANCELED:   return "Operation canceled";
        default:          return "Unknown error";
    }
}
