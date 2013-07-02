#include "signals.hpp"

#ifndef _WIN32
# include <unistd.h>
# include <sys/wait.h>
#endif // ifndef _WIN32
#include <signal.h>

namespace logoprism {
  namespace utils {

    namespace signals {

      static sig_atomic_t volatile kill_received = false;

      static void terminate_application(int) {
        kill_received = true;
      }

      static void terminate_with_backtrace(int signal_number) {
#ifndef _WIN32

        // fork and exec gdb to generate nice backtraces. If gdb is not available, this will fail silently.
        // fork, execve, waitpid, signal and raise are POSIX 'Asynchronous Signal Safe' functions which can safely be used in a signal handler.
        static char const* const gdb_backtrace_command[] = { "sh", "-c", "ps -p $$ -o ppid= | xargs gdb -batch -ex \"thread apply all bt\" -p 2>/dev/null", NULL };

        if (fork() == 0)
          execve("/bin/sh", (char**) gdb_backtrace_command, NULL);

        else
          waitpid(-1, NULL, 0);

#endif // ifndef _WIN32

        // restore default signal handler and raise it again.
        signal(signal_number, SIG_DFL);
        raise(signal_number);
      }

      void kill() {
        raise(SIGINT);
      }

      bool is_killed() {
        return kill_received;
      }

      void install() {
        signal(SIGINT, &terminate_application);
        signal(SIGTERM, &terminate_application);

        signal(SIGSEGV, &terminate_with_backtrace);
        signal(SIGABRT, &terminate_with_backtrace);
        signal(SIGILL, &terminate_with_backtrace);
        signal(SIGFPE, &terminate_with_backtrace);
      }

    }

  }
}
