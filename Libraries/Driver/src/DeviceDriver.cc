
#include <Driver.h>
#include <System.h>
#include <Types.h>

static L4_ThreadId_t tid IS_PERSISTENT;
static L4_Msg_t msg IS_PERSISTENT;

bool Recovered = false;

stat_t DeviceDriver::MainLoop() {
    L4_MsgTag_t     tag;
    stat_t          err = ERR_UNKNOWN;
   
    /* Have we returned from a restored failed request? */
    /* Then we have to reprocess the last message */
    if (Recovered) {
        goto restore;
    }

begin:
    tag = L4_Wait(&tid);
    
    for (;;) {
        if (L4_IpcFailed(tag)) {
            switch (Ipc::ErrorCode()) {
                case ERR_IPC_TIMEOUT:
                case ERR_IPC_TIMEOUT_SEND:
                case ERR_IPC_TIMEOUT_RECV:
                case ERR_IPC_NO_PARTNER:
                    goto begin;
                case ERR_IPC_CANCELED:
                case ERR_IPC_ABORTED:
                    err = Ipc::ErrorCode();
                    goto exit;
                case ERR_IPC_OVERFLOW:
                default:
                    goto begin;
            }
        }

        L4_Store(tag, &msg);
restore:
        err = Service(tid, msg);
        if (err != ERR_NONE) {
            System.Print(System.WARN,
                         "%s: Error processing message %lx from %.8lX\n",
                         Name(), L4_Label(tag), tid.raw);
            break;
        }
        L4_Load(&msg);
        tag = L4_ReplyWait(tid, &tid);
        Recovered = false;
    }
exit:
    return err;
}

