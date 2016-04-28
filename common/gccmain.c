#include <stdio.h>
#include <fcntl.h>
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

#define LOG(args...) //printf(args)


typedef VMINT (*vm_get_sym_entry_t)(char* symbol);
vm_get_sym_entry_t vm_get_sym_entry;

#define RESERVED_HEAP 1024*64             // heap reserved for C usage

unsigned int g_memory_size = 1024 * 800;  // heap for lua usage, will be adjusted
int g_memory_size_b = 0;                  // adjusted heap for C usage
static void* g_base_address = NULL;       // base address of the lua heap

extern void vm_main();

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
            vm_log_info("Init memory success, base: %#08x, size: %d, heap: %d", (caddr_t)g_base_address, g_memory_size, g_memory_size_b);
        }
    }

    prev_heap = heap;

    if(heap + incr > g_base_address + g_memory_size) {
        vm_log_fatal("Not enough memory");
    }
    else {
    	heap += incr;
    }

    return (caddr_t)prev_heap;
}

//---------------------------------------------
extern int link(char* old_name, char* new_name)
{
    VMWCHAR ucs_oldname[32], ucs_newname[32];
    vm_chset_ascii_to_ucs2(ucs_oldname, 32, old_name);
    vm_chset_ascii_to_ucs2(ucs_newname, 32, new_name);
    //return vm_fs_rename(old_name, new_name);
    return vm_fs_rename(ucs_oldname, ucs_newname);
}

//----------------------------------------------
int _open(const char* file, int flags, int mode)
{
    int result;
    VMUINT fs_mode;
    VMWCHAR wfile_name[64];
    char file_name[64];
    char* ptr;

    if(file[1] != ':') {
        snprintf(file_name, sizeof(file_name), "C:\\%s", file);
        ptr = file_name;
    } else {
        ptr = (char *)file;
    }

    vm_chset_ascii_to_ucs2(wfile_name, 64, ptr);

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

    result = vm_fs_open(wfile_name, fs_mode, 0);
    LOG("_open(%s, 0x%X, 0x%X) - %d\n", file, flags, mode, result);
    return result;
}

//-------------------------
extern int _close(int file)
{
    LOG("_close(%d)\n", file);
    vm_fs_close(file);
    return 0;
}

//------------------------------------------
extern int _fstat(int file, struct stat* st)
{
    int size;
    st->st_mode = S_IFCHR;

    if(file < 3) {
        return 0;
    }

    if(vm_fs_get_size(file, &size) > 0) {
        st->st_size = size;
    }

    LOG("_fstat(%d, 0x%X) - size: %d\n", file, (int)st, size);
    return 0;
}

//--------------------------
extern int _isatty(int file)
{
    if(file < 3) {
        return 1;
    }

    LOG("_isatty(%d)\n", file);
    return 0;
}

//-------------------------------------------------
extern int _lseek(int file, int offset, int whence)
{
    int position;
    int result;

    vm_fs_seek(file, offset, whence + 1);
    result = vm_fs_get_position(file, &position);

    LOG("_lseek(%d, %d, %d) - %d\n", file, offset, whence, position);
    return position;
}

//---------------------------------------
__attribute__((weak)) int retarget_getc()
{
    return 0;
}

//--------------------------------------------
extern int _read(int file, char* ptr, int len)
{
    if(file < 3) {
        int i;
        for(i = 0; i < len; i++) {
            *ptr = retarget_getc();
            ptr++;
        }
        return len;
    } else {
        int read_bytes = len;
        int bytes;
        bytes = vm_fs_read(file, ptr, len, &read_bytes);
        LOG("_read(%d, %s, %d, %d) - %d\n", file, ptr, len, read_bytes, bytes);
        return bytes;
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
        VMUINT written_bytes;

        vm_fs_write(file, ptr, len, &written_bytes);
        LOG("_write(%d, %s, %d, %d)\n", file, ptr, len, written_bytes);
        return written_bytes;
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

    while (g_base_address == NULL) {
    	g_base_address = vm_malloc(g_memory_size);
        if (g_base_address == NULL) {
        	g_memory_size -= 1024;
        }
        else break;
    }
    vm_free(g_base_address);
    g_base_address = NULL;
    g_memory_size_b = RESERVED_HEAP;
    g_memory_size -= RESERVED_HEAP;
	g_base_address = vm_malloc(g_memory_size);

    while ((g_base_address_b == NULL) && (g_memory_size_b > 0)) {
    	g_base_address_b = vm_malloc(g_memory_size_b);
        if (g_base_address_b == NULL) {
        	g_memory_size_b -= 1024;
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

    /*while (vm_time_ust_get_count() < 10000000) {
    	vm_thread_sleep(10);
    }*/
    vm_main();
}


#define VM_DCL_PIN_MODE_MAX 10

typedef struct {
    VMINT32 gpio;
    VMINT32 eint;
    VMINT32 adc;
    VMINT32 pwm;
    VMCHAR mux[VM_DCL_PIN_MODE_MAX];
} VM_DCL_PIN_MUX;

#define VM_DCL_PIN_TABLE_SIZE 21

static const VM_DCL_PIN_MUX pinTable[VM_DCL_PIN_TABLE_SIZE] = {
    { VM_PIN_P0, 0, 0, VM_DCL_PWM_4, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0 } },						//GPIO3   PWM1
    { VM_PIN_P1, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO27  SCK
    { VM_PIN_P2, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO28  MOSI
    { VM_PIN_P3, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, VM_DCL_PIN_MODE_SPI, 0, 0, 0, 0, 0 } },									//GPIO29  MISO
    { VM_PIN_P4, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO19
    { VM_PIN_P5, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO43  SCL
    { VM_PIN_P6, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_I2C, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO44  SDA
    { VM_PIN_P7, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO10  URXD1
    { VM_PIN_P8, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0, 0 } },									//GPIO11  UTSD1
    { VM_PIN_P9, 1, 15, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0, 0, 0, 0, 0, 0, 0 } },				//GPIO1   EINT1  ADC15
    { VM_PIN_P10, 2, 13, 0, { VM_DCL_PIN_MODE_GPIO, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_ADC, 0, 0, 0, 0, 0, 0, 0 } },				//GPIO2   EINT2  ADC13
    { VM_PIN_P11, 23, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0, 0, 0 } },								//AGPI52  EINT23
    { VM_PIN_P12, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0 } },								//GPIO17  UTXD2
    { VM_PIN_P13, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0 } },								//GPIO15  UART1_CTS(in)
    { VM_PIN_P14, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_UART, 0, 0, 0, 0, 0, 0, 0 } },								//GPIO12  UARTRXD2
    { VM_PIN_P15, 11, 0, VM_DCL_PWM_1, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, VM_DCL_PIN_MODE_PWM, 0, 0, 0, 0, 0, 0 } },	//GPIO13  EINT11 PWM0
    { VM_PIN_P16, 13, 0, VM_DCL_PWM_1, { VM_DCL_PIN_MODE_GPIO, 0, VM_DCL_PIN_MODE_EINT, 0, 0, 0, 0, 0, 0, 0 } },					//GPIO18  EINT13
    { VM_PIN_P17, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO47
    { VM_PIN_P18, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO48
    { VM_PIN_P19, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } },													//GPIO49
    { VM_PIN_P20, 0, 0, 0, { VM_DCL_PIN_MODE_GPIO, 0, 0, 0, 0, 0, 0, 0, 0, 0 } }													//GPIO50
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

