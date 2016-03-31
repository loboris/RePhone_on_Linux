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

#ifndef VMGRAPHIC_IMAGE_SDK_H
#define VMGRAPHIC_IMAGE_SDK_H

#ifdef __cplusplus
extern "C" {
#endif
#include "vmtype.h"
#include "vmgraphic.h"

    /* image property */
    typedef struct
    {
        VMINT width;                            /* image width */
        VMINT height;                           /* image height */
    } vm_graphic_image_property_t;


    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_draw_image_memory
     * DESCRIPTION
     *  Draws an image saved in memory to the frame.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     *  frame:          [IN] Frame pointer
     *  x:              [IN] X offset of start position
     *  y:              [IN] Y offset of start position
     *  image_data:     [IN] Data pointer
     *  image_length:   [IN] Length of image data
     *  frame_index:    [IN] Frame index of the image,
     *                  this is only meaningful on animated image, please fill "0" for other
     *                  image types.
     * RETURNS
     *  VM_RESULT refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_image_memory(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMUINT8* image_data,
                                           VMINT image_length, VMINT frame_index);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_draw_image_file
     * DESCRIPTION
     *  Draws an image file to the frame, this will replace the original frame
     *  content.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     *  frame :      [IN] Frame pointer
     *  x :          [IN] X offset of start position
     *  y :          [IN] Y offset of start position
     *  filename :   [IN] File name
     *  frame_index: [IN] Frame index of the image, this is for animated
     *                    images only, set to "0" for any other image types.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_image_file(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMWSTR file_name,
                                         VMINT frame_index);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_draw_resized_image_memory
     * DESCRIPTION
     *  Draws an image saved in memory to the frame resizing it to the specified size.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     *  frame :             [IN] Frame pointer
     *  x :                 [IN] X offset of start position
     *  y :                 [IN] Y offset of start position
     *  resized_width :     [IN] Resized width
     *  resized_height :    [IN] Resized height
     *  image_data :        [IN] Data pointer
     *  image_length :      [IN] Length of image data
     *  frame_index:        [IN] Frame index of the image, this is for animated
     *                           images only, set to "0" for any other image types.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_resized_image_memory(vm_graphic_frame_t* frame, VMINT x, VMINT y,
            VMINT resized_width, VMINT resized_height, VMUINT8* image_data, VMINT image_length,
            VMINT frame_index);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_draw_resized_image_file
     * DESCRIPTION
     *  Draws an image file to the frame resizing it to the specified size.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     *  frame :                 [IN] Frame pointer
     *  x :                     [IN] X offset of start position
     *  y :                     [IN] Y offset of start position
     *  resized_width :         [IN] Resized width
     *  resized_height :        [IN] Resized height
     *  file_name :             [IN] File name
     *  frame_index:            [IN] Frame index of the image, this is for animated
     *                           images only, set to "0" for any other image types.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_resized_image_file(vm_graphic_frame_t* frame, VMINT x, VMINT y,
            VMINT resized_width, VMINT resized_height, VMWSTR file_name, VMINT frame_index);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_image_memory_frame_number
     * DESCRIPTION
     *  Gets the frame number of an image saved in memory.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     * image_data :     [IN] Image data pointer.
     * image_length :   [IN] Image data size.
     * frame_number:    [OUT] Frame number of the image, this is only for
     *                        animated images, "1" will be returned for other image types.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_get_image_memory_frame_number(VMUINT8* image_data, VMINT image_length, VMINT* frame_number);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_image_file_frame_number
     * DESCRIPTION
     *  Gets the frame number of an image file.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     * file_name :           [IN]  Image file path.
     * frame_number :        [OUT] Frame number, the "frame" here is different from the vm_graphic_frame_t.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_get_image_file_frame_number(VMWSTR file_name, VMINT* frame_number);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_image_memory_property
     * DESCRIPTION
     *  Gets the property of an image saved in memory.
     *  The image format can be ABM / AB2 / JPEG / GIF / PNG / BMP
     * PARAMETERS
     * image_data :             [IN] Image data pointer.
     * image_length :           [IN] Image data size
     * frame_index:             [IN] Frame index of the image, this is for animated
     *                               images only, set to "0" for any other image types.
     * image_property :         [OUT] Image property
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_get_image_memory_property(VMUINT8* image_data, VMINT image_length, VMINT frame_index,
            vm_graphic_image_property_t* image_property);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_image_file_property
     * DESCRIPTION
     *  Gets the property of an image file.
     * PARAMETERS
     * file_name :           [IN]  Image file path.
     * image_property :      [OUT] Image property.
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_get_image_file_property(VMWSTR file_name, vm_graphic_image_property_t* image_property);

#ifdef _cplusplus
}
#endif
#endif /* VMGRAPHIC_IMAGE_SDK_H */
