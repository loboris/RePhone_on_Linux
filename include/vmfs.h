/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2005-2015
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/

#ifndef VMFS_SDK_H
#define VMFS_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"
#include "vmdatetime.h"

#ifndef VM_FS_MAX_PATH_LENGTH
#define VM_FS_MAX_PATH_LENGTH					(260)
#endif

typedef VMINT VM_FS_HANDLE;

/*
 * File attribution.
 */
typedef enum
{
    VM_FS_ATTRIBUTE_READ_ONLY    = 0x01,      /* Read only. */
    VM_FS_ATTRIBUTE_HIDDEN       = 0x02,      /* Hidden file  */
    VM_FS_ATTRIBUTE_SYSTEM       = 0x04,      /* System file  */
    VM_FS_ATTRIBUTE_VOLUME       = 0x08,      /* Volume file  */
    VM_FS_ATTRIBUTE_DIRECTORY    = 0x10,      /* Directory  */
    VM_FS_ATTRIBUTE_ARCHIVE      = 0x20,      /* Archive  */
    VM_FS_ATTRIBUTE_LONG_NAME    = 0x0F,      /* Long name  */
    VM_FS_ATTRIBUTE_MAX          = 0x7F       /* Forces the compiler to use 1-byte integer for enum. */
} VM_FS_ATTRIBUTE;


/* File information */
typedef struct
{
    VMWCHAR filename[VM_FS_MAX_PATH_LENGTH + 1]; /* Full path of a file */
    VMUINT size; /* File size */
} vm_fs_info_t;

/* File information */
typedef struct
{
    /* Filename without path */
    VMWCHAR full_filename[VM_FS_MAX_PATH_LENGTH + 1];
    /* Filename character array by 8.3 format encoding with UCS-2, and the last character may not be '\0' */
    VMCHAR filename[8];
    /* File extension character array by 8.3 format encoding with UCS-2, and the last character may not be '\0' */
    VMCHAR           extension[3];    /* File attributes */
    VMBYTE           attributes;
    vm_date_time_t   create_datetime; /* Created time */
    vm_date_time_t	 modify_datetime; /* Modified time */
    VMUINT           file_size;       /* File size */
    VMUINT           drive;           /* Drive */
    VMUINT           stamp;           /* Stamp */
} vm_fs_info_ex_t;

/* Disk information */
typedef struct
{
    /* A zero-terminated <color Red><b>Short File Name</b></color> with the volume's label. */
    VMCHAR            label[24];

    /* The drive letter in <color Red><b>UPPER</b></color> case. */
    VMCHAR            drive_letter;

    /* Indicates the disk is in write protection or not. 1 means in protection, 0 means not. */
    VMUINT8           write_protect;

    /* Reserved fields */
    VMCHAR            reserved[2];

    /* The volume's serial number. */
    VMUINT32          serial_number;

    /* The LBA address of the logical drive's boot record. For diskettes, this value will be 0. */
    VMUINT32          first_physical_sector;

    /* 2, 16, or 32 for FAT-12, FAT-16, or FAT-32 */
    VMUINT32          fat_type;

    /* The number of FATs on the volume. */
    VMUINT32          fat_count;

    /* The maximum number of directory entries on the volume. */
    VMUINT32          max_directory_entries;

    /* The sector size. This value will usually be 512. */
    VMUINT32          bytes_per_sector;

    /* Specifies the size of the smallest unit of storage that can be allocated to a file in sectors. */
    VMUINT32          sectors_per_cluster;

    /* Number of clusters for file storage on the volume. */
    VMUINT32          total_clusters;

    /* The number of clusters, which are marked bad and are unavailable for file storage. */
    VMUINT32          bad_clusters;

    /* The number of clusters currently available. */
    VMUINT32          free_cluster;

    /*
     * The number of files on the volume including directories, but not counting
     * the root directory and files with an allocated file size of 0.
     */
    VMUINT32          files;

    /*
     * The number of contiguous cluster chains. On a completely unfragmented
     * volume, this value would be identical to Files.
     */
    VMUINT32          file_chains;

    /*
     * The number of contiguous cluster chains of free clusters. On a completely
     * unfragmented volume, this value would be 1.
     */
    VMUINT32          free_chains;

    /*
     * The maximum allocated file size for a newly allocated contiguous file in
     * clusters. On a completely unfragmented volume, this value would be
     * identical to FreeClusters.
     */
    VMUINT32          largest_free_chain;
} vm_fs_disk_info_t;

/* vm_fs_get_disk_info Parameter */
typedef enum
{
    /*
     * Returns all fields in structure vm_fs_get_disk_info except FreeClusters,
     * BadClusters, Files, FileChains, FreeChains, LargestFreeChain.
     * <color Red><b>This flag never requires a FAT scan.</b></color>
     */
    VM_FS_DISK_INFO_FLAG_BASIC_INFO     =  0x00000001L,

    /*
     * Returns field FreeClusters. This flags may require FS to scan the complete
     * FAT if the amount of free space is not known. In this case,
     * FS_DISK_INFO_FAT_STATISTICS is returned in addition to FS_DISK_INFO_FREE_SPACE.
     */
    VM_FS_DISK_INFO_FLAG_FREE_SPACE     =  0x00000002L,

    /*
     * Returns fields BadClusters, Files, FileChains, FreeChains, LargestFreeChain.
     * This flag will always cause FS to scan the complete FAT.
     */
    VM_FS_DISK_INFO_FLAG_FAT_STATISTICS =  0x00000004L,

    /*
     * Gets volume label of the target drive. The maximum length is 11 wide characters.
     * <color Red><b>This option must be used with FS_DISK_INFO_BASIC_INFO.</b></color>
     */
    VM_FS_DISK_INFO_FLAG_VOLUME_LABEL   =  0x00000008L,

    VM_FS_DISK_INFO_FLAG_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */

} VM_FS_DISK_INFO_FLAG;


/* File mode.*/
typedef enum
{
    VM_FS_MODE_READ                = 1,         /* Opens file with file read-only mode. */
    VM_FS_MODE_WRITE               = 2,         /* Opens file with read-write mode. */
    VM_FS_MODE_CREATE_ALWAYS_WRITE = 4,         /* Opens file with read-write mode, or creates the file if not exists. */
    VM_FS_MODE_APPEND              = 8,         /* Opens file with append mode. */
    VM_FS_MODE_MAX                 = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_FS_MODE;


/* File base. */
typedef enum
{
    VM_FS_BASE_BEGINNING    = 1,         /* Based at the beginning. */
    VM_FS_BASE_CURRENT      = 2,         /* Based at the current position.*/
    VM_FS_BASE_END          = 3,         /* Based at the end. */
    VM_FS_BASE_POSITION_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_FS_BASE_POSITION;


/*
 * File system return values.
 */
typedef enum
{
    VM_FS_SUCCESS                     =    0,    /* No error */
    VM_FS_RESERVED                    =   -1,    /* Reserved, not used */
    VM_FS_PARAMETER                   =   -2,    /* User */
    VM_FS_INVALID_FILENAME            =   -3,    /* User */
    VM_FS_DRIVE_NOT_FOUND             =   -4,    /* User or Fatal */
    VM_FS_TOO_MANY_FILES              =   -5,    /* User or Normal: Exceeded the maximum number of file handles or more than 256 files in sort */
    VM_FS_NO_MORE_FILES               =   -6,    /* Normal */
    VM_FS_WRONG_MEDIA                 =   -7,    /* Fatal */
    VM_FS_INVALID_FILE_SYSTEM         =   -8,    /* Fatal */
    VM_FS_FILE_NOT_FOUND              =   -9,    /* User or Normal */
    VM_FS_INVALID_FILE_HANDLE         =  -10,    /* User or Normal */
    VM_FS_UNSUPPORTED_DEVICE          =  -11,    /* User */
    VM_FS_UNSUPPORTED_DRIVE_FUNCTION  =  -12,    /* User or Fatal */
    VM_FS_CORRUPTED_PARTITION_TABLE   =  -13,    /* Fatal */
    VM_FS_TOO_MANY_DRIVES             =  -14,    /* Not used */
    VM_FS_INVALID_FILE_POS            =  -15,    /* User */
    VM_FS_ACCESS_DENIED               =  -16,    /* User or Normal */
    VM_FS_STRING_BUFFER_TOO_SMALL     =  -17,    /* User */
    VM_FS_GENERAL_FAILURE             =  -18,    /* Normal */
    VM_FS_PATH_NOT_FOUND              =  -19,    /* User */
    VM_FS_FAT_ALLOC                   =  -20,    /* Fatal: Disk crashes */
    VM_FS_ROOT_DIR_FULL               =  -21,    /* Normal */
    VM_FS_DISK_FULL                   =  -22,    /* Normal */
    VM_FS_TIMEOUT                     =  -23,    /* Normal: FS_CloseMSDC with non-block */
    VM_FS_BAD_SECTOR                  =  -24,    /* Normal: NAND flash bad block */
    VM_FS_DATA                        =  -25,    /* Normal: NAND flash bad block */
    VM_FS_MEDIA_CHANGED               =  -26,    /* Normal */
    VM_FS_SECTOR_NOT_FOUND            =  -27,    /* Fatal */
    VM_FS_ADDRESS_MARK_NOT_FOUND      =  -28,    /* Not used */
    VM_FS_DRIVE_NOT_READY             =  -29,    /* Normal */
    VM_FS_WRITE_PROTECTION            =  -30,    /* Normal: Only for MSDC */
    VM_FS_DMA_OVERRUN                 =  -31,    /* Not used */
    VM_FS_CRC                         =  -32,    /* Not used */
    VM_FS_DEVICE_RESOURCE             =  -33,    /* Fatal: Device crashes */
    VM_FS_INVALID_SECTOR_SIZE         =  -34,    /* Fatal */
    VM_FS_OUT_OF_BUFFERS              =  -35,    /* Fatal */
    VM_FS_FILE_EXISTS                 =  -36,    /* User or Normal */
    VM_FS_LONG_FILE_POS               =  -37,    /* User: FS_Seek new pos over sizeof int */
    VM_FS_FILE_TOO_LARGE              =  -38,    /* User: filesize + pos over sizeof int */
    VM_FS_BAD_DIR_ENTRY               =  -39,    /* Fatal */
    VM_FS_ATTRIBUTE_CONFLICT          =  -40,    /* User: Can't specify FS_PROTECTION_MODE and FS_NONBLOCK_MOD */
    VM_FS_CHECKDISK_RETRY             =  -41,    /* System: Don't care */
    VM_FS_LACK_OF_PROTECTION_SPACE    =  -42,    /* Fatal: Device crashes */
    VM_FS_SYSTEM_CRASH                =  -43,    /* Normal */
    VM_FS_FAIL_GET_MEMORY             =  -44,    /* Normal */
    VM_FS_READ_ONLY                   =  -45,    /* User or Normal */
    VM_FS_DEVICE_BUSY                 =  -46,    /* Normal */
    VM_FS_ABORTED                     =  -47,    /* Normal */
    VM_FS_QUOTA_OVER_DISK_SPACE       =  -48,    /* Normal: Configuration Mistake */
    VM_FS_PATH_OVER_LEN               =  -49,    /* Normal */
    VM_FS_APP_QUOTA_FULL              =  -50,    /* Normal */
    VM_FS_VF_MAP                      =  -51,    /* User or Normal */
    VM_FS_DEVICE_EXPORTED             =  -52,    /* User or Normal */
    VM_FS_DISK_FRAGMENT               =  -53,    /* Normal */
    VM_FS_DIRCACHE_EXPIRED            =  -54,    /* Normal */
    VM_FS_QUOTA_USAGE_WARNING         =  -55,    /* Normal or Fatal: System Drive Free Space Not Enough */
    VM_FS_ERROR_DIRDATA_LOCKED        =  -56,    /* Normal */
    VM_FS_INVALID_OPERATION           =  -57,    /* Normal */
    VM_FS_ERROR_VF_PARENT_CLOSED      =  -58,    /* Virtual file's parent is closed */
    VM_FS_ERROR_UNSUPPORTED_SERVICE   =  -59,    /* The specified FS service is closed in this project. */
    VM_FS_ERROR_INVALID_JOB_ID        =  -81,
    VM_FS_ERRoR_ASYNC_JOB_NOT_FOUND   =  -82,
    VM_FS_MSDC_MOUNT                  = -100,    /* Normal */
    VM_FS_MSDC_READ_SECTOR            = -101,    /* Normal */
    VM_FS_MSDC_WRITE_SECTOR           = -102,    /* Normal */
    VM_FS_MSDC_DISCARD_SECTOR         = -103,    /* Normal */
    VM_FS_MSDC_PRESNET_NOT_READY      = -104,    /* System */
    VM_FS_MSDC_NOT_PRESENT            = -105,    /* Normal */
    VM_FS_EXTERNAL_DEVICE_NOT_PRESENT = -106,    /* Normal */
    VM_FS_HIGH_LEVEL_FORMAT           = -107,    /* Normal */
    VM_FS_CARD_BATCHCOUNT_NOT_PRESENT = -110,    /* Normal */
    VM_FS_FLASH_MOUNT                 = -120,    /* Normal */
    VM_FS_FLASH_ERASE_BUSY            = -121,    /* Normal: Only for non-block mode */
    VM_FS_NAND_DEVICE_NOT_SUPPORTED   = -122,    /* Normal: Configuration Mistake */
    VM_FS_FLASH_OTP_UNKNOWERR         = -123,    /* User or Normal */
    VM_FS_FLASH_OTP_OVERSCOPE         = -124,    /* User or Normal */
    VM_FS_FLASH_OTP_WRITEFAIL         = -125,    /* User or Normal */
    VM_FS_FDM_VERSION_MISMATCH        = -126,    /* System */
    VM_FS_FLASH_OTP_LOCK_ALREADY      = -127,    /* User or Normal */
    VM_FS_FDM_FORMAT                  = -128,    /* The format of the disk content is not correct */
    VM_FS_FDM_USER_DRIVE_BROKEN       = -129,    /* User drive unrecoverably broken */
    VM_FS_FDM_SYS_DRIVE_BROKEN        = -130,    /* System drive unrecoverably broken */
    VM_FS_FDM_MULTIPLE_BROKEN         = -131,    /* Multiple unrecoverably broken*/
    VM_FS_LOCK_MUTEX_FAIL             = -141,    /* System: Don't care */
    VM_FS_NO_NONBLOCKMODE             = -142,    /* User: Try to call non-block mode other than NOR flash */
    VM_FS_NO_PROTECTIONMODE           = -143,    /* User: Try to call protection mode other than NOR flash */
    VM_FS_DISK_SIZE_TOO_LARGE         = (VM_FS_MSDC_MOUNT),     /* Normal */
    VM_FS_MINIMUM_RESULT              = -65536,  /* 0xFFFF0000 */

    VM_FS_RESULT_MAX                  = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */

} VM_FS_RESULT;


/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_first
 * DESCRIPTION
 *    Finds the first file or folder under the specified path, and return a handle.
 *    This function supports the wild character (*) in the search pattern.
 *    The size and the full path of the first file or folder (no size if it is a
 *    folder) is returned through the out parameter of the vm_fs_info_t object.
 *
 *    A handle of this find function is returned to the caller, which can be used
 *    for getting the next file or folder by vm_fs_find_next().
 *
 *    The caller of this function should call vm_fs_find_close() to release the
 *    resource after it is done with the handle.
 * PARAMETERS
 *    path_name: [IN]  The path, encoded in UCS-2.
 *    file_info: [OUT] Contains the full path and size of the found file or folder.
 * RETURNS
 *    If a positive integer is returned, it is the handle for finding the next file or folder.
 *    If a negative integer is returned, then refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_FS_HANDLE vm_fs_find_first(VMWSTR path_name, vm_fs_info_t* file_info);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_next
 * DESCRIPTION
 *    Gets the next file or folder from the VM_FS_HANDLE object,which is returned
 *    from the vm_fs_find_first(). If exists, the size and the full path of the
 *    file or folder (no size if it is a folder) is returned through the out
 *    parameter of the vm_fs_info_t object. If there is no file or folder exists,
 *    the VM_FS_NO_MORE_FILES error code is returned
 * PARAMETERS
 *    handle:    [IN]  The handle, returned from vm_fs_find_first().
 *    file_info: [OUT] Contains the full path and size of the found file or folder.
 * RETURNS
 *    VM_FS_RESULT
 *****************************************************************************/
VM_FS_RESULT vm_fs_find_next(VM_FS_HANDLE handle, vm_fs_info_t* file_info);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_close
 * DESCRIPTION
 *    Closes the file finding process and releases the resource.
 * PARAMETERS
 *    handle: [IN] The handle, returned from vm_fs_find_first().
 * RETURN VALUES
 *    VM_FS_RESULT
 *****************************************************************************/
VM_FS_RESULT vm_fs_find_close(VM_FS_HANDLE handle);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_removable_drive_letter
 * DESCRIPTION
 *    Gets the drive letter of the removable disk, which normally is the memory card.
 * PARAMETERS
 *    void
 * RETURNS
 *   The drive letter of the removable disk. A negative number means no removable disk.
 *****************************************************************************/
VMCHAR vm_fs_get_removable_drive_letter(void);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_internal_drive_letter
 * DESCRIPTION
 *    Gets drive letter of the removable disk, which normally is the memory card.
 * PARAMETERS
 *    void
 * RETURNS
 *   The drive letter of the removable disk. A negative number means no removable drive.
 *****************************************************************************/
VMCHAR vm_fs_get_internal_drive_letter(void);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_disk_free_space
 * DESCRIPTION
 *    Checks the available free space on the specified drive. It returns the size
 *    of the available free space in bytes.
 * PARAMETERS
 *    drive_letter: [IN] The drive letter string terminated by '\0'.
 * RETURNS
 *    The size of the available free space in bytes. If the return value is 0, it
 *    indicates that the specified drive does not exist.
 *
 *    If the return value is 0xFFFFFFFF, it indicates the size of the available
 *    free space exceeds the limitation of the return type (VMUINT, 32-bit) of this
 *    function can represent. In this case, the vm_fs_get_disk_info() should be used.
 *
 * EXAMPLE
 * <code>
 * VMINT sys_drive = -1;
 * VMUINT sys_drive_size = 0;
 * if ((sys_drive = vm_fs_get_system_drive()) >= 0)
 * {
 *     VMCHAR drive_str[2] = {0};
 *
 *     sprintf(drive_str, "%c", (VMCHAR)sys_drive);
 *     sys_drive_size = vm_fs_get_disk_free_space(vm_ucs2_string(drive_str));
 * }
 * </code>
 *****************************************************************************/
VMUINT vm_fs_get_disk_free_space(VMCHAR drive_letter);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_disk_info
 * DESCRIPTION
 *    Gets disk information of a logical drive.
 * PARAMETERS
 *    drive_letter: [IN]  Must be a valid filename, e.g., a root directory name.
 *                        Only the drive information (possibly the current default drive) is determined.
 *    disk_info:    [OUT] Disk information obtained from calling this function.
 *    flags:        [IN]  A VM_FS_DISK_FLAG to specify what specific information to get.
 * RETURNS
 *    A positive integer indicates a success. A negative integer indicates a failure.
 *    Refer to VM_FS_RESULT for error code, if fails.
 *****************************************************************************/
VM_RESULT vm_fs_get_disk_info(VMCHAR drive_letter, vm_fs_disk_info_t* fs_disk, VM_FS_DISK_INFO_FLAG flags);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_first_ex
 * DESCRIPTION
 *    Similar to the vm_fs_find_first(), except it obtains more information, as
 *    specified in vm_fs_info_ex_t, for the file or folder from the system.
 *
 *    It finds the first file or folder under the specified path, and return a handle.
 *    This function supports the wild character (*) in the search pattern.
 *    The file or folder information is returned through the out parameter of the
 *    vm_fs_info_ex_t object.
 *
 *    A handle of this find function is returned to the caller, which can be used
 *    for getting the next file or folder by calling vm_fs_find_next_ex().
 *
 *    The caller of this function should call vm_fs_find_close_ex() to release the
 *    resource after it is done with the handle.
 * PARAMETERS
 *    path_name: [IN]  The path, encoded in UCS-2.
 *    file_info: [OUT] The file information obtained by calling this function.
 * RETURNS
 *    If a positive integer is returned, it is the handle for finding the next file or folder.
 *    If a negative integer is returned, then refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_FS_HANDLE vm_fs_find_first_ex(VMWSTR path_name, vm_fs_info_ex_t* file_info);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_next_ex
 * DESCRIPTION
 *    Gets the information of the next file or folder from the VM_FS_HANDLE object,
 *    which is returned from the vm_fs_find_first_ex(). If the next file or folder
 *    exists, the information of the file or folder is returned through the out
 *    parameter of the vm_fs_info_ex_t object. If no file or folder exists,
 *    the VM_FS_NO_MORE_FILES error code is returned.
 * PARAMETERS
 *    handle:    [IN]  The handle, returned from vm_fs_find_first_ex().
 *    file_info: [OUT] The file information obtained by calling this function.
 * RETURNS
 *    If 0 is returned, it indicates the next file or folder was found. If a
 *    negative number is returned, it indicates the end of the find and no more
 *    file or folder information will be returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_find_next_ex(VM_FS_HANDLE handle, vm_fs_info_ex_t* file_info);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_find_close_ex
 * DESCRIPTION
 *    Close the file finding process and releases the resource.
 * PARAMETERS
 *    handle: [IN] The handle, returned from vm_fs_find_first_ex().
 * RETURN VALUES
 *    VM_FS_RESULT
 *****************************************************************************/
VM_FS_RESULT vm_fs_find_close_ex(VM_FS_HANDLE handle);

/* DOM-NOT_FOR_SDK-BEGIN */
typedef struct
{
    VMBYTE* destination;
    VMUINT destination_size;
    VMBYTE* source;
    VMUINT source_size;
} vm_fs_zip_t;

typedef void* (*app_malloc_callback)(VMUINT items, VMUINT size);
typedef void (*app_free_callback)(void* address);
VM_RESULT vm_fs_uncompress(const vm_fs_zip_t* info, app_malloc_callback app_malloc, app_free_callback app_free);
/* DOM-NOT_FOR_SDK-END */


typedef enum
{
    VM_FS_COMMIT_ERROR = -1,
    VM_FS_OPEN_ERROR = -2,
    VM_FS_NAME_ERROR = -1,
    VM_FS_OK = 0,
    VM_FS_ERROR_MAX = 0x7FFFFFFF /* Forces the compiler to use 4-byte integer for enum. */
} VM_FS_ERROR;

/*****************************************************************************
 * FUNCTION
 *    vm_fs_open
 * DESCRIPTION
 *    Opens a file. It opens the file as a binary file, if the is_binary is VM_TRUE.
 *    If the is_binary is VM_FALSE, it opens the file as an ASCII file.
 * PARAMETERS
 *    filename:  [IN] The name of the file to be opened. It is the full path encoded in UCS-2.
 *    mode:      [IN] Specifies the mode to open the file.
 *    is_binary: [IN] Specifies if the file to be opened as a binary or an ASCII file.
 * RETURNS
 *    The file handle is returned, if the open operation is successful. If fails,
 *    a negative integer is returned. Refer to the VM_FS_RESULT for error code,
 *    if it fails.
 *****************************************************************************/
VM_FS_HANDLE vm_fs_open(VMCWSTR filename, VM_FS_MODE mode, VMBOOL is_binary);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_close
 * DESCRIPTION
 *    Closes a file that is opened by vm_fs_open().
 * PARAMETERS
 *    handle: [IN] The file handle of the file to be closed. The handle is returned by vm_fs_open().
 * RETURNS
 *    VM_FS_RESULT
 *****************************************************************************/
VM_FS_RESULT vm_fs_close(VM_FS_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_read
 * DESCRIPTION
 *    Reads the file through the file handle.
 * PARAMETERS
 *    handle: [IN]  The file handle of the file to be read.
 *    buffer: [OUT] Stores the data read from the file.
 *    size:   [IN]  The size of the buffer in bytes.
 *    bytes:  [OUT] The actual number of bytes read from the file.
 * RETURNS
 *    If it returns a positive value, the value is the number of bytes of data
 *    read from the file. If the return value is negative, it indicates an error
 *    has occurred. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_read(VM_FS_HANDLE handle, void* buffer, VMUINT size, VMUINT* bytes);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_write
 * DESCRIPTION
 *    Writes data in the buffer to a file pointed to by the file handle.
 * PARAMETERS
 *    handle: [IN]  The file handle where the data to be written to.
 *    buffer: [IN]  The buffer containing data to be written to a file.
 *    size:   [IN]  The size of the data in the buffer in bytes.
 *    bytes:  [IN]  The actual number of bytes written to the file.
 * RETURNS
 *    If it returns a positive value, the value is the number of bytes of data
 *    written to the file. If the return value is negative value, it indicates
 *    an error has occurred. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_write(VM_FS_HANDLE handle, const void* buffer, VMUINT size, VMUINT* bytes);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_flush
 * DESCRIPTION
 *    Commits the I/O cache data to a file.
 * PARAMETERS
 *    handle: [IN] The file handle to commit the I/O cache data to.
 * RETURNS
 *    It returns 0, when the flush operation succeeds. If it fails, a negative
 *    value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_flush(VM_FS_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_seek
 * DESCRIPTION
 *    Moves the file pointer to a specific position. If the specified position
 *    exceeds the size of the file, an error will be returned.
 *    If the function succeeds, the return value is the new file pointer value,
 *
 *    Moving the file pointer before the beginning of the file is an error.
 *    However, moving the file pointer beyond the current file size is supported.
 *    In this case, the file is extended. The data between the previous file size
 *    and the new file size is undefined. This method is used to extend a file,
 *    which is a much faster way than actually writing data to it.
 * PARAMETERS
 *    handle: [IN]  The file handle of the file to be operated on.
 *    offset: [IN]  The number of bytes to move.
 *    base:   [IN]  The base position where the file pointer will move from.
 *                  VM_FS_BASE_BEGINNING means from begin of file;
 *                  VM_FS_BASE_CURRENT means from current position;
 *                  VM_FS_BASE_END means from end of file.
 * RETURNS
 *    If succeeds, a positive integer is returned, which is the new position
 *    (base +/- offset) of the file pointer points to.
 *    If a negative integer is returned, refer to VM_FS_RESULT for error details.
 *    If the value of the resulting file pointer is larger than 2^(31)-1,
 *    VM_FS_LONG_FILE_POS is returned.
 *****************************************************************************/
VM_RESULT vm_fs_seek(VM_FS_HANDLE handle, VMINT offset, VM_FS_BASE_POSITION base);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_position
 * DESCRIPTION
 *    Gets the current position of the file pointer of the file, pointed to by the
 *    specified file handle.
 * PARAMETERS
 *    handle:   [IN]  The handle of the file to get the current position from.
 *    position: [OUT] The current position of the file pointer, returned after calling this function.
 * RETURNS
 *    A positive integer of the current file pointer position. If a negative integer
 *    is returned, refer to VM_FS_RESULT for error details.
 *****************************************************************************/
VM_RESULT vm_fs_get_position(VM_FS_HANDLE handle, VMUINT* position);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_is_eof
 * DESCRIPTION
 *    Checks if the current file pointer has reached to the end of the file.
 * PARAMETERS
 *    handle: [IN] The file handle of the file to be checked against.
 * RETURNS
 *    VMBOOL to indicates if the current pointer has reached to the file or not.
 *    If the current pointer has reached to the end of the file, VM_TRUE is returned.
 *    Otherwise, VM_FALSE is returned.
*****************************************************************************/
VMBOOL vm_fs_is_eof(VM_FS_HANDLE handle);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_size
 * DESCRIPTION
 *    Obtains the size of the file in bytes. The file is specified by the file handle.
 * PARAMETERS
 *    handle: [IN]  The file handle of the file.
 *    size:   [OUT] The size of the file specified by the file handle in bytes,
 *                  an output parameter of this function.
 * RETURNS
 *    It returns 0, if the size of the file was obtained successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_get_size(VM_FS_HANDLE handle, VMUINT* size);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_delete
 * DESCRIPTION
 *    Deletes the file as specified by the full path of the filename.
 * PARAMETERS
 *    filename: [IN] The full path of the filename of the file to be deleted, encoded in UCS-2.
 * RETURNS
 *    Delete file successfully or failed
 * RETURN VALUES
 *    It returns 0, if the file was deleted successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_delete(VMCWSTR filename);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_rename
 * DESCRIPTION
 *    Renames the file. Both old_name and new_name are full path of a filename,
 *    encoded in UCS-2.
 * PARAMETERS
 *    old_name: [IN]  The full path of the old filename, encoded in UCS-2.
 *    new_name: [IN]  The full path of the new filename, encoded in UCS-2.
 * RETURNS
 *    rename file successfully or failed
 * RETURN VALUES
 *    It returns 0, if the file was renamed successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_rename(VMCWSTR old_name, VMCWSTR new_name);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_create_directory
 * DESCRIPTION
 *    Creates a new directory with specified path name, encoded in UCS-2.
 * PARAMETERS
 *    dir_name: [IN] The full path name of the directory to be created, encoded in UCS-2.
 * RETURNS
 *    create directory successfully or failed
 * RETURN VALUES
 *    It returns 0, if the directory was created successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_create_directory(VMCWSTR dir_name);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_remove_directory
 * DESCRIPTION
 *    Removes a directory as specified by full path name, encoded in UCS-2.
 * PARAMETERS
 *    dir_name: [IN] The full path name of the directory to be removed, encoded in UCS-2.
 * RETURNS
 *    remove directory successfully or failed
 * RETURN VALUES
 *    It returns 0, if the directory was removed successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_remove_directory(VMCWSTR dir_name);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_set_attributes
 * DESCRIPTION
 *    Sets the attributes for a file or directory as specified full path filename,
 *    encoded in UCS-2. The attributes can not include VM_FS_ATTRIBUTE_VOLUME,
 *    VM_FS_ATTRIBUTE_DIRECTORY, or VM_FS_LONGNAME_ATTRIBUTE. The value of attributes
 *    is the result of bitwise OR operation of individual attribute.
 * PARAMETERS
 *    filename:   [IN] The full path name of the file or directory, encoded in UCS-2.
 *    attributes: [IN] The bitwise OR of individual attribute.
 * RETURNS
 *    It returns 0, if the attributes were set successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_set_attributes(VMCWSTR filename, VM_FS_ATTRIBUTE attributes);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_get_attributes
 * DESCRIPTION
 *    Retrieves the attributes of a file or directory, as specified by the full
 *    path name, encoded in UCS-2. The value of the attributes is the result of
 *    bitwise OR operation of individual attribute.
 * PARAMETERS
 *    filename:    [IN]  The full path filename encoded UCS-2.
 *    attributes : [OUT] The retrieved attributes, if this function returns successfully.
 * RETURNS
 *    It returns 0, if the attributes were retrieved successfully. If fails,
 *    a negative value is returned. Refer to VM_FS_RESULT for error code.
 *****************************************************************************/
VM_RESULT vm_fs_get_attributes(VMCWSTR filename, VM_FS_ATTRIBUTE* attributes);


/*****************************************************************************
* FUNCTION
 *    vm_fs_app_data_open
 * DESCRIPTION
 *    Opens the application data file in system drive disk.
 * PARAMETERS
 *    mode:      [IN] Specifies the file open mode:
 *                      MODE_READ: Read-Only.
 *                      MODE_WRITE: Read-Write.
 *                      MODE_CREATE_ALWAYS_WRITE: Read-Write mode, or creates if the file not exists.
 *    is_binary: [IN] Specifies if the file to be opened as a binary or as an ASCII file.
 *                    If it is VM_TRUE, it indicates the file to be opened as binary file.
 *                    If it is VM_FALSE, it indicates the file to be opened as ASCII file.
 * RETURNS
 *    The file handle is returned, if the open operation is successful. If fails,
 *    a negative integer is returned. Refer to the VM_FS_RESULT for error code,
 *    if it fails.
 *****************************************************************************/
VM_FS_HANDLE vm_fs_app_data_open(VM_FS_MODE mode, VMBOOL binary);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_app_data_seek
 * DESCRIPTION
 *    Moves the system data file pointer to a specific position. If the specified
 *    position exceeds the size of the file, an error will be returned.
 *    If the function succeeds, the return value is the new file pointer value,
 *
 *    Moving the file pointer before the beginning of the file is an error.
 *    However, moving the file pointer beyond the current file size is supported.
 *    In this case, the file is extended. The data between the previous file size
 *    and the new file size is undefined. This method is used to extend a file,
 *    which is a much faster way than actually writing data to it.
 * PARAMETERS
 *    handle: [IN]  The file handle of the file to be operated on.
 *    offset: [IN]  The number of bytes to move.
 *    base:   [IN]  The base position where the file pointer will move from.
 *                  VM_FS_BASE_BEGINNING means from begin of file;
 *                  VM_FS_BASE_CURRENT means from current position;
 *                  VM_FS_BASE_END means from end of file.
 * RETURNS
 *    If succeeds, a positive integer is returned, which is the new position
 *    (base +/- offset) of the file pointer points to.
 *    If a negative integer is returned, refer to VM_FS_RESULT for error details.
 *    If the value of the resulting file pointer is larger than 2^(31)-1,
 *    VM_FS_LONG_FILE_POS is returned.
 *****************************************************************************/
VM_RESULT vm_fs_app_data_seek(VM_FS_HANDLE handle, VMINT offset, VM_FS_BASE_POSITION base);


/*****************************************************************************
* FUNCTION
 *    vm_fs_app_data_read
 * DESCRIPTION
 *    Reads the application data from the data file, pointed to by the file handle.
 * PARAMETERS
 *    handle: [IN]  The handle of the data file to read the data from.
 *    data:   [OUT] The data buffer used to stored the data read from the data file.
 *    size:   [IN]  The size of the data buffer.
 *    bytes:  [OUT] The number of bytes actually read from the data file in this operation.
 * RETURNS
 *    A positive integer, which is the bytes of data being read from the data file.
 *    If it fails or reach to the end of the data file, the value of 0 is returned.
 *
 *****************************************************************************/
VMINT vm_fs_app_data_read(VM_FS_HANDLE handle, void* data, VMUINT size, VMUINT* bytes);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_app_data_write
 * DESCRIPTION
 *    Writes data at the current position of the data file, pointed to by the handle.
 * PARAMETERS
 *    handle: [IN]  The handle of the data file to write data to.
 *    data:   [IN]  The data buffer where the data to be written to the data file.
 *    size:   [IN]  The size of the data buffer.
 *    bytes:  [OUT] The number of bytes actually written to the data file in this operation.
 * RETURNS
 *    A positive integer, which is the bytes of data being written to the data file.
 *    If it fails, the value of 0 is returned.
 *****************************************************************************/
VMINT vm_fs_app_data_write(VM_FS_HANDLE handle, const void* data, VMUINT size, VMUINT* bytes);

/*****************************************************************************
* FUNCTION
 *    vm_fs_app_data_close
 * DESCRIPTION
 *    Closes the application data file file.
 * PARAMETERS
 *    handle: [IN] The handle of the application data file to be closed.
 * RETURNS:
 *    void
 *****************************************************************************/
void vm_fs_app_data_close(VM_FS_HANDLE handle);

/*****************************************************************************
* FUNCTION
 *    vm_fs_app_data_delete
 * DESCRIPTION
 *    Deletes the content in the application data space.
 * RETURNS
 *    It returns 0, if it deletes the application data successfully.
 *    It returns a non-zero value, if it fails to delete the application data.
 *****************************************************************************/
VM_RESULT vm_fs_app_data_delete(void);

/*****************************************************************************
* FUNCTION
 *    vm_fs_app_data_get_free_space
 * DESCRIPTION
 *    Checks the size of the current available free space for the application data.
 *    Each application (a VXP file) is assigned with a storage space for the
 *    application data. The application is responsible for not storing data that
 *    exceeds the size of the assigned space.
 * PARAMETERS
 *    void
 * RETURNS
 *    The size of the free space in bytes.
*****************************************************************************/
VMINT vm_fs_app_data_get_free_space(void);


typedef VMINT VM_FS_COPY_HANDLE;
typedef VMINT VM_FS_COPY_ACTION;
/*****************************************************************************
 * FUNCTION
 *    vm_fs_copy_callback
 * DESCRIPTION
 *    Defines a callback function type for file copy.
 * PARAMETERS
 *    act:          [IN] The copy action. See VM_FS_COPY_ACTION.
 *    total_bytes:  [IN] The size of the source file to be copied from, in bytes.
 *    copied_bytes: [IN] The number of bytes being copied
 *    copy_handle:  [IN] The handle of the file (destination) being copied to.
 * RETURNS
 *    void
 *****************************************************************************/
typedef void (*vm_fs_copy_callback)(VM_FS_COPY_ACTION action, VMUINT32 total_bytes,
                                    VMUINT32 copied_bytes, VM_FS_COPY_HANDLE copy_handle);


/*****************************************************************************
 * FUNCTION
 *    vm_fs_copy
 * DESCRIPTION
 *    Copies a file from source to destination. The filenames of source and destination
 *    are full path encoded in UCS-2.
 * PARAMETERS
 *    destination: [IN] The full path of destination where the file is copied to, encoded in UCS-2.
 *    source:      [IN] The full path of source where the file is copied from, encoded in UCS-2.
 *    callback:    [IN] callback of the file copy operation.
 * RETURNS
 *    VM_FS_RESULT. The typical error codes are:
 *    (-2):  Bad parameter.
 *    (-3):  Either source full path or destination full path is wrong.
 *    (-19): Cannot find source full path specified.
 *    (-36): destination full path exists.
 *    (-44): Cannot get memory to do the operation
 *    (-46): Busy.
 *    (-47): This action has been aborted in the user's callback function.
 *****************************************************************************/
VM_FS_RESULT vm_fs_copy(VMCWSTR destination, VMCWSTR source, vm_fs_copy_callback callback);

/*****************************************************************************
 * FUNCTION
 *    vm_fs_copy_abort
 * DESCRIPTION
 *    Aborts a file copy. This function should be only invoked inside the callback
 *    function of the file copy operation.
 * PARAMETERS
 *    copy_handle: [IN] The handle of the file copy. This handle is passed to this
 *                      function through the vm_fs_copy_callback().
 * RETURNS
 *    VM_FS_RESULT
 *****************************************************************************/
VM_FS_RESULT vm_fs_copy_abort(VM_FS_HANDLE copy_handle);



#ifdef __cplusplus
}
#endif

#endif /* VMFS_SDK_H */

