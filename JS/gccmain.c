
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#if defined(__GNUC__) /* GCC CS3 */
#include <sys/stat.h>
#endif

#include "vmtype.h"
#include "vmlog.h"
#include "vmmemory.h"
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmdcl_pwm.h"
#include "vmfs.h"
#include "vmchset.h"
#include "vmdatetime.h"
#include "vmthread.h"
#include "vmboard.h"

#include "shell.h"


#define LOG(args...) //printf(args)


typedef VMINT (*vm_get_sym_entry_t)(char* symbol);
vm_get_sym_entry_t vm_get_sym_entry;

#define RESERVED_HEAP 1024*64				// initial size of heap reserved for C usage

unsigned int g_memory_size = 1024 * 800;	// heap for lua usage, ** will be adjusted **
int g_memory_size_b = 0;					// adjusted heap for C usage
static void* g_base_address = NULL;			// base address of the lua heap
int g_reserved_heap = RESERVED_HEAP;
int g_max_heap_inc = 0;

extern void vm_main();
extern int retarget_getc(int tmo);
extern void _get_sys_vars(int doset, void *heapsz, void *wdgtmo);

int __g_errno = 0;


char *__tzname[2] = { (char *) "CET", (char *) "CET" };
int __daylight = 1;
long int __timezone = 1L;


//-----------------------
void __cxa_pure_virtual()
{
    while(1)
        ;
}

//------------
int* __errno()
{
    return &__g_errno;
}

//----------------------------
extern caddr_t _sbrk(int incr)
{
    static void* heap = NULL;
    static void* base = NULL;
    void* prev_heap;

    if(heap == NULL) {
        base = (unsigned char*)g_base_address;
        if(base == NULL) {
            vm_log_fatal("malloc failed");
        } else {
            heap = base;
            //vm_log_info("Init memory success, base: %#08x, size: %d, heap: %d", (caddr_t)g_base_address, g_memory_size, g_memory_size_b);
        }
    }

    prev_heap = heap;

    if ((heap + incr) > (g_base_address + g_memory_size)) {
        vm_log_fatal("Not enough memory, requested %d bytes from %p", incr, heap);
    }
    else {
    	heap += incr;
    }
    if (incr > g_max_heap_inc) g_max_heap_inc = incr;

    return (caddr_t)prev_heap;
}

//---------------------------------------------
extern int link(char* old_name, char* new_name)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FRENAME;
    g_CCparams.cpar1 = old_name;
    g_CCparams.cpar2 = new_name;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//----------------------------------------------
int _open(const char* file, int flags, int mode)
{
    VMUINT fs_mode;

    if(flags & O_CREAT) {
        fs_mode = VM_FS_MODE_CREATE_ALWAYS_WRITE;
    } else if((flags & O_RDWR) || (flags & O_WRONLY)) {
        fs_mode = VM_FS_MODE_WRITE;
    } else {
        fs_mode = VM_FS_MODE_READ;
    }

    if(flags & O_APPEND) {
        fs_mode |= VM_FS_MODE_APPEND;
    }

    g_fcall_message.message_id = CCALL_MESSAGE_FOPEN;
    g_CCparams.cpar1 = (char *)file;
    g_CCparams.ipar1 = fs_mode;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//-------------------------
extern int _close(int file)
{
    //vm_fs_close(file);
    g_fcall_message.message_id = CCALL_MESSAGE_FCLOSE;
    g_CCparams.ipar1 = file;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//------------------------------------------
extern int _fstat(int file, struct stat* st)
{
    int size;
    st->st_mode = S_IFCHR;

    if(file < 3) return 0;

    /*if(vm_fs_get_size(file, &size) > 0) {
        st->st_size = size;
    }*/
    g_fcall_message.message_id = CCALL_MESSAGE_FSIZE;
    g_CCparams.ipar1 = file;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    if (g_shell_result > 0) {
        st->st_size = size;
    }
    return 0;
}

//--------------------------
extern int _isatty(int file)
{
    if (file < 3) return 1;
    return 0;
}

//-------------------------------------------------
extern int _lseek(int file, int offset, int whence)
{
    g_fcall_message.message_id = CCALL_MESSAGE_FSEEK;
    g_CCparams.ipar1 = file;
    g_CCparams.ipar2 = offset;
    g_CCparams.ipar3 = whence + 1;
    vm_thread_send_message(g_main_handle, &g_fcall_message);
    // wait for call to finish...
    vm_signal_wait(g_shell_signal);

    return g_shell_result;
}

//----------------------------------------------
__attribute__((weak)) int retarget_getc(int tmo)
{
    return 0;
}

//--------------------------------------------
extern int _read(int file, char* ptr, int len)
{
    if(file < 3) {
        int i, ch;
        i = 0;
        while (i < len) {
            ch = retarget_getc(0);
            if (ch >= 0) {
                // backspace key
                if (ch == 0x7f || ch == 0x08) {
                    if (i > 0) {
                        fputs("\x08 \x08", stdout);
                        fflush(stdout);
                        i--;
                        ptr--;
                    }
                    continue;
                }
                // EOF(ctrl+d)
                else if (ch == 0x04) break;
                // end of line
                if (ch == '\r') ch = '\n';
                if (ch == '\n') {
                	*ptr++ = ch;
                	i++;
                	break;
                }
                // other control character or not an acsii character
                if (ch < 0x20 || ch >= 0x80) continue;

                // echo
    			fputc(ch, stdout);
    			fflush(stdout);
            	*ptr++ = ch;
            	i++;
            }
            else vm_thread_sleep(10);
        }
        for (int j=0; j<i; j++) {
            fputs("\x08 \x08", stdout);
        }
        fflush(stdout);
        return i;
    }
    else {
        g_fcall_message.message_id = CCALL_MESSAGE_FREAD;
        g_CCparams.ipar1 = file;
        g_CCparams.ipar2 = len;
        g_CCparams.cpar1 = ptr;
        vm_thread_send_message(g_main_handle, &g_fcall_message);
        // wait for call to finish...
        vm_signal_wait(g_shell_signal);

        return g_shell_result;
    }
}

//----------------------------------------------
__attribute__((weak)) void retarget_putc(char c)
{
}

//---------------------------------------------
extern int _write(int file, char* ptr, int len)
{
    if(file < 3) {
        int i;

        for(i = 0; i < len; i++) {
            retarget_putc(*ptr);
            ptr++;
        }
        return len;
    } else {
        g_fcall_message.message_id = CCALL_MESSAGE_FWRITE;
        g_CCparams.ipar1 = file;
        g_CCparams.ipar2 = len;
        g_CCparams.cpar1 = ptr;
        vm_thread_send_message(g_main_handle, &g_fcall_message);
        // wait for call to finish...
        vm_signal_wait(g_shell_signal);

        return g_shell_result;
    }
}

//---------------------------
extern void _exit(int status)
{
    for(;;)
        ;
}

//---------------------------------
extern void _kill(int pid, int sig)
{
    return;
}

//----------------------
extern int _getpid(void)
{
    return -1;
}

//-----------------------------
int __cxa_guard_acquire(int* g)
{
    return !*(char*)(g);
};

//------------------------------
void __cxa_guard_release(int* g)
{
    *(char*)g = 1;
};

typedef void (**__init_array)(void);

//void __libc_init_array(void);

//-----------------------------------------------------------------------------------
void gcc_entry(unsigned int entry, unsigned int init_array_start, unsigned int count)
{
    unsigned int i;
    void* g_base_address_b = NULL;

    __init_array ptr;
    vm_get_sym_entry = (vm_get_sym_entry_t)entry;

    // Get system variables
    _get_sys_vars(1,0,0);

    // get maximum possible heap size
    while (g_base_address == NULL) {
    	g_base_address = vm_malloc(g_memory_size);
        if (g_base_address == NULL) {
        	g_memory_size -= 256;
        }
        else break;
    }
    vm_free(g_base_address);

    g_base_address = NULL;
    g_memory_size_b = g_reserved_heap;
    g_memory_size -= g_reserved_heap;

    // allocate heap size for Lua
	g_base_address = vm_malloc(g_memory_size);

	// find maximum C heap size
    while ((g_base_address_b == NULL) && (g_memory_size_b > 0)) {
    	g_base_address_b = vm_malloc(g_memory_size_b);
        if (g_base_address_b == NULL) {
        	g_memory_size_b -= 256;
        }
        else break;
    }
    if (g_memory_size_b < 0) g_memory_size_b = 0;
    if (g_base_address_b != NULL) vm_free(g_base_address_b);
    g_base_address_b = NULL;

    ptr = (__init_array)init_array_start;
    for(i = 1; i < count; i++) {
        ptr[i]();
    }

    // start main prog
    vm_main();
}

//===========================================================================

#define VM_DCL_PIN_MODE_MAX 10

typedef struct {
    VMINT32 gpio;
    VMINT32 eint;
    VMINT32 adc;
    VMINT32 pwm;
    VMCHAR mux[VM_DCL_PIN_MODE_MAX];
} VM_DCL_PIN_MUX;

#define VM_DCL_PIN_TABLE_SIZE 25

static const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
    { VM_PIN_P0, 0, 12, VM_DCL_PWM_4, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_ADC, 0, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0 } },	//GPIO3   PWM1 ADC12
    { VM_PIN_P1, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO27  SCK
    { VM_PIN_P2, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO28  MOSI
    { VM_PIN_P3, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO29  MISO
    { VM_PIN_P4, 0, 0, VM_DCL_PWM_4, { VM_DCL_PIN_MODE_GPIO, 0, 0, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0, 0 } },						//GPIO19  PWM1
    { VM_PIN_P5, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO43  SCL
    { VM_PIN_P6, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO44  SDA
    { VM_PIN_P7, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO10  UART1_RX
    { VM_PIN_P8, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO11  UART1_TX
    { VM_PIN_P9, 1, 13, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0 } },	//GPIO1   EINT1  ADC13  UART3_TX
    { VM_PIN_P10, 2, 11, VM_DCL_PWM_1, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0 } },	//GPIO2   EINT2  ADC11
    //{ VM_PIN_P11, 15, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0 } },								//GPIO25  EINT15
    { VM_PIN_P12, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO17  RED_LED
    { VM_PIN_P13, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO15  GREEN_LED
    { VM_PIN_P14, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO12  BLUE_LED
    { VM_PIN_P15, 11, 0, VM_DCL_PWM_1, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0, 0 } },	//GPIO13  EINT11 PWM0
    { VM_PIN_P16, 13, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0, 0, 0 } },								//GPIO18  EINT13
    { VM_PIN_P17, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO47
    { VM_PIN_P18, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO48
    { VM_PIN_P19, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO49
    { VM_PIN_P20, 22, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0, 0, 0 } },								//GPIO50  EINT22
    { VM_PIN_P21, 20, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0 , 0, 0, 0, 0, 0, 0} },								//GPIO46  EINT20
    { VM_PIN_P22, 16, 0, 0, {VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0 , 0, 0, 0, 0, 0, 0} },								//GPIO30  EINT16
    { VM_PIN_P23, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO38
    { VM_PIN_P24, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO39
    { VM_PIN_P25, 23, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0, 0, 0 } }								//GPIO52  EINT23
};

//--------------------------------------------------------------------
VM_DCL_STATUS vm_dcl_config_pin_mode(VMUINT pin, VM_DCL_PIN_MODE mode)
{
    VM_DCL_HANDLE gpio_handle;
    VMINT i, j;
    VM_DCL_STATUS status = VM_DCL_STATUS_FAIL;

    for(i = 0; i < VM_DCL_PIN_TABLE_SIZE; i++) {
        if(pinTable[i].gpio == pin) {
            break;
        }
    }
    if(i >= VM_DCL_PIN_TABLE_SIZE)
        return status;

    gpio_handle = vm_dcl_open(VM_DCL_GPIO, pin);

    for(j = 0; j < VM_DCL_PIN_MODE_MAX; j++) {
        if(pinTable[i].mux[j] == mode) {
            vm_dcl_control(gpio_handle, 4 + j, NULL);
            status = VM_DCL_STATUS_OK;
            break;
        }
    }
    vm_dcl_close(gpio_handle);
    return status;
}

//----------------------------------
VMUINT vm_dcl_pin_to_pwm(VMUINT pin)
{
    VMINT i;

    for(i = 0; i < VM_DCL_PIN_TABLE_SIZE; i++) {
        if(pinTable[i].gpio == pin) {
            return pinTable[i].pwm;
        }
    }

    return 0;
}

//--------------------------------------
VMUINT vm_dcl_pin_to_channel(VMUINT pin)
{
    VMINT i;

    for(i = 0; i < VM_DCL_PIN_TABLE_SIZE; i++) {
        if(pinTable[i].gpio == pin) {
            return pinTable[i].adc;
        }
    }

    return 0;
}

//-----------------------------------
VMUINT vm_dcl_pin_to_eint(VMUINT pin)
{
    VMINT i;

    for(i = 0; i < VM_DCL_PIN_TABLE_SIZE; i++) {
        if(pinTable[i].gpio == pin) {
            return pinTable[i].eint;
        }
    }

    return 0;
}

typedef struct {
    VMINT gpio;
    VMINT mode;
    VM_DCL_HANDLE handle; 
} VM_DCL_PIN_HANDLE;

//----------------------------------------------------------------
static VM_DCL_PIN_HANDLE pinHandleTable[VM_DCL_PIN_TABLE_SIZE] = {
    { VM_PIN_P0, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P1, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P2, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P3, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P4, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P5, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P6, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P7, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P8, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P9, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P10, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P11, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P12, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P13, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P14, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P15, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P16, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P17, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P18, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P19, 0, VM_DCL_HANDLE_INVALID },
    { VM_PIN_P20, 0, VM_DCL_HANDLE_INVALID }
};

//-------------------------------------------------
int gpio_get_handle(int pin, VM_DCL_HANDLE *handle)
{
    int i;
    for (i = 0; i < VM_DCL_PIN_TABLE_SIZE; i++) {
        if (pinHandleTable[i].gpio == pin) {
            if (pinHandleTable[i].handle == VM_DCL_HANDLE_INVALID) {
                pinHandleTable[i].handle = vm_dcl_open(VM_DCL_GPIO, pin);
            }
            *handle = pinHandleTable[i].handle;
            
            return pinHandleTable[i].handle;
        }
    }
    
    return -1;
}

