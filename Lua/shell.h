
#ifndef __SHELL_H__
#define __SHELL_H__

#include "vmthread.h"
#include "lua.h"

#define SHELL_MESSAGE_ID        326
#define SHELL_MESSAGE_QUIT      328

VM_THREAD_HANDLE g_main_handle;
vm_thread_message_t g_shell_message;

VMINT32 shell_thread(VM_THREAD_HANDLE thread_handle, void* user_data);
void l_message (const char *pname, const char *msg);
void shell_docall(lua_State *L);

#endif // __SHELL_H__
