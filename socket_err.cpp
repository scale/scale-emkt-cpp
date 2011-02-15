#include "global.h"


const char * get_socket_error(int err_no) {

	switch (err_no) {
		case EADDRNOTAVAIL: return "The specified address is not available from the local machine (EADDRNOTAVAIL)";
		case EAFNOSUPPORT:  return "The specified address is not a valid address for the address family of the specified socket (EAFNOSUPPORT)";
		case EALREADY:      return "A connection request is already in progress for the specified socket (EALREADY)";
		case EBADF:         return "The socket argument is not a valid file descriptor (EBADF)";
		case ECONNREFUSED:  return "The target address was not listening for connections or refused the connection request (ECONNREFUSED)";
		case EFAULT:        return "The address parameter can not be access (EFAULT)";
		case EINPROGRESS:   return "O_NONBLOCK is set for the file descriptor for the socket and the connection cannot be immediately established; the connection will be established asynchronously (EINPROGRESS)";
		case EINTR:         return "The attempt to establish a connection was interrupted by delivery of a signal that was caught; the connection will be established asynchronously (EINTR)";
		case EISCONN:       return "The specified socket is connection-mode and is already connected (EISCONN)";
		case ENETUNREACH:   return "No route to the network is present (ENETUNREACH)";
		case ENOTSOCK:      return "The socket argument does not refer to a socket (ENOTSOCK)";
		case EPROTOTYPE:    return "The specified address has a different type than the socket bound to the specified peer address (EPROTOTYPE)";
		case ETIMEDOUT:     return "The attempt to connect timed out before a connection was made (ETIMEDOUT)";
		case EACCES:        return "Search permission is denied for a component of the path prefix; or write access to the named socket is denied (EACCES)";
		case EADDRINUSE:    return "Attempt to establish a connection that uses addresses that are already in use (EADDRINUSE)";
		case ECONNRESET:    return "Remote host reset the connection request (ECONNRESET)";
		case EHOSTUNREACH:  return "The destination host cannot be reached (probably because the host is down or a remote router cannot reach it) (EHOSTUNREACH)";
		case EINVAL:        return "The address_len argument is not a valid length for the address family; or invalid address family in sockaddr structure (EINVAL)";
		case ENAMETOOLONG:  return "Pathname resolution of a symbolic link produced an intermediate result whose length exceeds {PATH_MAX} (ENAMETOOLONG)";
		case ENETDOWN:      return "The local interface used to reach the destination is down (ENETDOWN)";
		case ENOBUFS:       return "No buffer space is available (ENOBUFS)";
		case ENOSR:         return "There were insufficient STREAMS resources available to complete the operation (ENOSR)";
		case EOPNOTSUPP:    return "The socket is listening and can not be connected (EOPNOTSUPP)";

		default: return "unknown error";
	}

}


