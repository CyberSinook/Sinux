#include "signal.h"
#include "../proc/process.h"
#include "../proc/scheduler.h"
#include "../../lib/string.h"

int
signal_send(int pid, int sig)
{
    if (sig < 1 || sig >= NSIG) return -1;
    process_t *p = proc_find((pid_t)pid);
    if (!p) return -1;
    p->sig_pending |= (1ULL << sig);
    if (p->state == PROC_SLEEPING || p->state == PROC_WAITING)
        sched_wake(p);
    return 0;
}

void
signal_deliver(void)
{
    process_t *p = proc_current();
    if (!p || !p->sig_pending) return;

    for (int sig = 1; sig < NSIG; sig++) {
        if (!(p->sig_pending & (1ULL << sig))) continue;
        if (p->sig_blocked & (1ULL << sig)) continue;

        p->sig_pending &= ~(1ULL << sig);

        sigaction_t *act = &p->sig_actions[sig];

        if (act->handler == SIG_IGN) continue;

        if (act->handler == SIG_DFL) {
            switch (sig) {
            case SIGKILL:
            case SIGTERM:
            case SIGINT:
            case SIGPIPE:
                proc_exit(p, 128 + sig);
                sched_yield();
                return;
            case SIGSTOP:
                p->state = PROC_SLEEPING;
                sched_yield();
                return;
            default:
                break;
            }
        } else {
            act->handler(sig);
        }
    }
}

int
signal_set_handler(int sig, sigaction_t *act)
{
    if (sig < 1 || sig >= NSIG) return -1;
    if (sig == SIGKILL || sig == SIGSTOP) return -1;
    process_t *p = proc_current();
    p->sig_actions[sig] = *act;
    return 0;
}
