
///
/// @brief  Device driver template
/// @file   Include/arc/driver.h
/// @since  March 2008
///

//$Id: Driver.h 429 2008-11-01 02:24:02Z hro $

#ifndef ARC_DRIVER_H
#define ARC_DRIVER_H

#include <Ipc.h>
#include <Types.h>

class DeviceDriver
{
protected:
    ///
    /// Deals with requests to the driver.  This method has to be reentrant.
    ///
    /// @param tid      the requesting thread
    /// @param msg      the request
    ///
    virtual stat_t Service(const L4_ThreadId_t& tid, L4_Msg_t& msg) = 0;
    
public:
    ///
    /// The service main loop
    ///
    stat_t MainLoop();
    
    ///
    /// Returns the unique name of the driver
    ///
    virtual const char *const Name() = 0;
    
    ///
    /// Initializes the peripheral and the driver itself.
    ///
    virtual stat_t Initialize() = 0;

    ///
    /// Recovers the driver.
    ///
    virtual stat_t Recover() = 0;


    ///
    /// Finalizes the peripheral and the driver.
    ///
    virtual stat_t Exit();

    ///
    /// Activates the device driver object.
    ///
    static void Activate(DeviceDriver& obj);
};

#define IS_PERSISTENT __attribute__ ((section(".pdata")))

/**
 * Can be used at any time by the driver to know whether the request
 * being processed is an attempt to recover
 */
extern bool Recovered;

#define ARC_DRIVER(CLASS)                       \
    int server_main(int argc, char* argv[], int state) {    \
        CLASS drv;                              \
        stat_t err = ERR_UNKNOWN;               \
        switch (state) {                        \
            case 0:                             \
                Recovered = false;              \
                err = drv.Initialize();         \
                break;                          \
            case 1:                             \
                Recovered = true;               \
                err = drv.Recover();            \
                break;                          \
            default:                            \
                break;                          \
        }                                       \
        if (err != ERR_NONE) {                  \
            return static_cast<int>(err);       \
        }                                       \
        drv.MainLoop();                         \
        err = drv.Exit();                       \
        return static_cast<int>(err);           \
    }

#define BEGIN_HANDLERS(CLASS)                                   \
    stat_t CLASS::Service(const L4_ThreadId_t& tid, L4_Msg_t& msg) {  \
        stat_t ret;                                             \
        switch (L4_Label(L4_MsgTag(&msg))) {

#define CONNECT_HANDLER(MSGID,HANDLER)                          \
            case MSGID: ret = HANDLER(tid, &msg); break;

#define END_HANDLERS                                                        \
            default:                                                        \
                System.Print(System.WARN,                                   \
                             "%s: Unknown message %lX from %.8lX\n",        \
                             Name(), L4_Label(L4_MsgTag(&msg)), tid.raw);   \
                ret = ERR_NOT_FOUND;                                        \
                break;                                                      \
        }                                                                   \
        return ret;                                                         \
    }

#endif // ARC_DRIVER_H

