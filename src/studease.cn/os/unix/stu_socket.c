/*
 * stu_socket.c
 *
 *  Created on: 2018��3��30��
 *      Author: Tony Lau
 */

#include <stu_config.h>
#include <stu_core.h>


/*
 * ioctl(FIONBIO) sets a non-blocking mode with the single syscall
 * while fcntl(F_SETFL, O_NONBLOCK) needs to learn the current state
 * using fcntl(F_GETFL).
 *
 * ioctl() and fcntl() are syscalls at least in FreeBSD 2.x, Linux 2.2
 * and Solaris 7.
 *
 * ioctl() in Linux 2.4 and 2.6 uses BKL, however, fcntl(F_SETFL) uses it too.
 */

#if (STU_HAVE_FIONBIO)

int
stu_nonblocking(stu_socket_t s) {
	int  nb;

	nb = 1;

	return ioctl(s, FIONBIO, &nb);
}

int
stu_blocking(stu_socket_t s) {
	int  nb;

	nb = 0;

	return ioctl(s, FIONBIO, &nb);
}

#endif


#if (STU_FREEBSD)

int
stu_tcp_nopush(stu_socket_t s) {
	int  tcp_nopush;

	tcp_nopush = 1;

	return setsockopt(s, IPPROTO_TCP, TCP_NOPUSH, (const void *) &tcp_nopush, sizeof(int));
}

int
stu_tcp_push(stu_socket_t s) {
	int  tcp_nopush;

	tcp_nopush = 0;

	return setsockopt(s, IPPROTO_TCP, TCP_NOPUSH, (const void *) &tcp_nopush, sizeof(int));
}

#elif (STU_LINUX)

int
stu_tcp_nopush(stu_socket_t s) {
	int  cork;

	cork = 1;

	return setsockopt(s, IPPROTO_TCP, TCP_CORK, (const void *) &cork, sizeof(int));
}

int
stu_tcp_push(stu_socket_t s) {
	int  cork;

	cork = 0;

	return setsockopt(s, IPPROTO_TCP, TCP_CORK, (const void *) &cork, sizeof(int));
}

#else

int
stu_tcp_nopush(stu_socket_t s) {
	return 0;
}

int
stu_tcp_push(stu_socket_t s) {
	return 0;
}
