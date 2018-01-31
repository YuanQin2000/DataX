/*
 * See the file LICENSE for redistribution information.
 *
 * Copyright (c) 2015-2018, Yuan Qin <yuanqin2000 at outlook dot com>
 * All rights reserved.
 *
 */

#include "Common/ErrorNo.h"
#include <cstring>
#include <errno.h>
#include "Tracker/Trace.h"

using std::strerror;

ErrorCode GetStandardErrorCode(int err)
{
    if (err >= EC_SUCCESS) {
        return static_cast<ErrorCode>(err);
    }

    ErrorCode ecCode;
    switch (err) {
    case 0:
        ecCode = EC_SUCCESS;
        break;

    case EBADFD:          // File descriptor in bad state
    case EBUSY:           // Device or resource busy (POSIX.1)
    case EIO:             // Input/output error (POSIX.1)
    case ENODEV:          // No such device (POSIX.1)
    case ENOENT:          // No such file or directory (POSIX.1)
    case ENOTTY:          // Inappropriate I/O:  control operation (POSIX.1)
    case EPIPE:           // Broken pipe (POSIX.1)
    case EREMOTEIO:       // Remote I/O:  error
    case ESTALE:          // Stale file handle (POSIX.1)
    case ESTRPIPE:        // Streams pipe error
        ecCode = EC_IO_ERROR;
        break;

    case ENOMEM:          // Not enough space (POSIX.1)
        ecCode = EC_NO_MEMORY;
        break;

    case ENETUNREACH:     // Network unreachable (POSIX.1)
    case ENETDOWN:        // Network is down (POSIX.1)
    case EHOSTDOWN:       // Host is down
    case EHOSTUNREACH:    // Host is unreachable (POSIX.1)
        ecCode = EC_CONNECT_FAILED;
        break;

    case E2BIG:           // Argument list too long (POSIX.1)
    case EACCES:          // Permission denied (POSIX.1)
    case EADDRINUSE:      // Address already in use (POSIX.1)
    case EADDRNOTAVAIL:   // Address not available (POSIX.1)
    case EAFNOSUPPORT:    // Address family not supported (POSIX.1)
    case EAGAIN:          // Resource temporarily unavailable (may be equal to EWOULDBLOCK) (POSIX.1)
    case EALREADY:        // Connection already in progress (POSIX.1)
    case EBADE:           // Invalid exchange
    case EBADF:           // Bad file descriptor (POSIX.1)
    case EBADMSG:         // Bad message (POSIX.1)
    case EBADR:           // Invalid request descriptor
    case EBADRQC:         // Invalid request code
    case EBADSLT:         // Invalid slot
    case ECANCELED:       // Operation cancelled (POSIX.1)
    case ECHILD:          // No child processes (POSIX.1)
    case ECHRNG:          // Channel number out of range
    case ECOMM:           // Communication error on send
    case ECONNABORTED:    // Connection aborted (POSIX.1)
    case ECONNREFUSED:    // Connection refused (POSIX.1)
    case ECONNRESET:      // Connection reset (POSIX.1)
    case EDEADLK:         // Resource deadlock avoided (POSIX.1), same as EDEADLOCK
    case EDESTADDRREQ:    // Destination address required (POSIX.1)
    case EDOM:            // Mathematics argument out of domain of function (POSIX.1, C99)
    case EDQUOT:          // Disk quota exceeded (POSIX.1)
    case EEXIST:          // File exists (POSIX.1)
    case EFAULT:          // Bad address (POSIX.1)
    case EFBIG:           // File too large (POSIX.1)
    case EIDRM:           // Identifier removed (POSIX.1)
    case EILSEQ:          // Illegal byte sequence (POSIX.1, C99)
    case EINPROGRESS:     // Operation in progress (POSIX.1)
    case EINTR:           // Interrupted function call (POSIX.1); see signal(7).
    case EINVAL:          // Invalid argument (POSIX.1)
    case EISCONN:         // Socket is connected (POSIX.1)
    case EISDIR:          // Is a directory (POSIX.1)
    case EISNAM:          // Is a named type file
    case EKEYEXPIRED:     // Key has expired
    case EKEYREJECTED:    // Key was rejected by service
    case EKEYREVOKED:     // Key has been revoked
    case EL2HLT:          // Level 2 halted
    case EL2NSYNC:        // Level 2 not synchronized
    case EL3HLT:          // Level 3 halted
    case EL3RST:          // Level 3 halted
    case ELIBACC:         // Cannot access a needed shared library
    case ELIBBAD:         // Accessing a corrupted shared library
    case ELIBMAX:         // Attempting to link in too many shared libraries
    case ELIBSCN:         // lib section in a.out corrupted
    case ELIBEXEC:        // Cannot exec a shared library directly
    case ELOOP:           // Too many levels of symbolic links (POSIX.1)
    case EMEDIUMTYPE:     // Wrong medium type
    case EMFILE:          // Too many open files (POSIX.1)
    case EMLINK:          // Too many links (POSIX.1)
    case EMSGSIZE:        // Message too long (POSIX.1)
    case EMULTIHOP:       // Multihop attempted (POSIX.1)
    case ENAMETOOLONG:    // File name too long (POSIX.1)
    case ENETRESET:       // Connection aborted by network (POSIX.1)
    case ENOBUFS:         // No buffer space available (POSIX.1 (XSI: STREAMS:  option))
    case ENODATA:         // No message is available on the STREAM:  head read queue (POSIX.1)
    case ENOEXEC:         // Exec format error (POSIX.1)
    case ENOKEY:          // Required key not available
    case ENOLCK:          // No locks available (POSIX.1)
    case ENOLINK:         // Link has been severed (POSIX.1)
    case ENOMEDIUM:       // No medium found
    case ENOMSG:          // No message of the desired type (POSIX.1)
    case ENONET:          // Machine is not on the network
    case ENOPKG:          // Package not installed
    case ENOPROTOOPT:     // Protocol not available (POSIX.1)
    case ENOSPC:          // No space left on device (POSIX.1)
    case ENOSR:           // No STREAM:  resources (POSIX.1 (XSI: STREAMS:  option))
    case ENOSTR:          // Not a STREAM:  (POSIX.1 (XSI: STREAMS:  option))
    case ENOSYS:          // Function not implemented (POSIX.1)
    case ENOTBLK:         // Block device required
    case ENOTCONN:        // The socket is not connected (POSIX.1)
    case ENOTDIR:         // Not a directory (POSIX.1)
    case ENOTEMPTY:       // Directory not empty (POSIX.1)
    case ENOTSOCK:        // Not a socket (POSIX.1)
    case ENOTSUP:         // Operation not supported (POSIX.1)
    case ENOTUNIQ:        // Name not unique on network
    case ENXIO:           // No such device or address (POSIX.1)
    case EOVERFLOW:       // Value too large to be stored in data type (POSIX.1)
    case EPERM:           // Operation not permitted (POSIX.1)
    case EPFNOSUPPORT:    // Protocol family not supported
    case EPROTO:          // Protocol error (POSIX.1)
    case EPROTONOSUPPORT: // Protocol not supported (POSIX.1)
    case EPROTOTYPE:      // Protocol wrong type for socket (POSIX.1)
    case ERANGE:          // Result too large (POSIX.1, C99)
    case EREMCHG:         // Remote address changed
    case EREMOTE:         // Object is remote
    case ERESTART:        // Interrupted system call should be restarted
    case EROFS:           // Read-only file system (POSIX.1)
    case ESHUTDOWN:       // Cannot send after transport endpoint shutdown
    case ESPIPE:          // Invalid seek (POSIX.1)
    case ESOCKTNOSUPPORT: // Socket type not supported
    case ESRCH:           // No such process (POSIX.1)
    case ETIME:           // Timer expired (POSIX.1 (XSI: STREAMS:  option))
    case ETIMEDOUT:       // Connection timed out (POSIX.1)
    case ETXTBSY:         // Text file busy (POSIX.1)
    case EUCLEAN:         // Structure needs cleaning
    case EUNATCH:         // Protocol driver not attached
    case EUSERS:          // Too many users
    case EXDEV:           // Improper link (POSIX.1)
    case EXFULL:          // Exchange full
    default:
        OUTPUT_ERROR_TRACE("Unknown error code: %d(%s)\n", err, strerror(err));
        ecCode = EC_UNKNOWN;
        break;
    }
    return ecCode;
}

const char* GetErrorPhrase(ErrorCode error)
{
    ASSERT(error > EC_MIN && error < EC_MAX);
    static const char* s_ErrorPhrase[] = {
        "NO Error",
        "In Progress",
        "Inactive",
        "Protocol Malformat Data",
        "Protocol Error",
        "Connect Refused",
        "I/O Error",
        "No Memory",
        "Illegal Parameters",
        "Unknown Error",
    };

    return s_ErrorPhrase[error - EC_SUCCESS];
}
