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

#ifndef VMGRAPHIC_FONT_SDK_H
#define VMGRAPHIC_FONT_SDK_H

#ifdef __cplusplus
extern "C" {
#endif
#include "vmtype.h"
#include "vmgraphic.h"

    /* font size */
    typedef enum
    {
        VM_GRAPHIC_SMALL_FONT,          /*Small font*/
        VM_GRAPHIC_MEDIUM_FONT,         /*Medium font*/
        VM_GRAPHIC_LARGE_FONT,          /*Large font*/
        VM_GRAPHIC_FONT_SIZE_MAX = 0x7FFFFFFF
    } VM_GRAPHIC_FONT_SIZE;

    /* character information in pixel */
    typedef struct
    {
        VMINT real_width;           /* The real width of the character, can be used for typeset */
        VMINT width;                /* The width of the character, not include padding */
        VMINT height;               /* The height of the character */
        VMINT ascent;               /* Base line */
        VMINT descent;              /* The length from the character bottom to the baseline */
    } vm_graphic_character_info_t;

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_draw_text
     * DESCRIPTION
     *  Draws the texts at the given position within the frame. The font / color / font_size are following the
     *  global settings set by vm_graphic_font_set_font() / vm_graphic_color() /
     *  vm_graphic_font_set_font_size(). If they are not set the default system font / block
     *  color and medium font will be used.
     * PARAMETERS
     * frame :  [IN] Frame pointer
     * x :      [IN] X offset of start position
     * y :      [IN] Y offset of start position
     * text :   [IN] Text encoded using ucs2 format
     * RETURNS
     *   VM_RESULT refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_text(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMWSTR text);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_draw_text_by_baseline
     * DESCRIPTION
     *  Draws the text to the frame according to the baseline. The font / color / font_size are
     *  following the global setting set by vm_graphic_font_set_font() /
     *  vm_graphic_color() / vm_graphic_font_set_font_size(). If not set the default
     *  system font / block color and medium font will be used.
     * PARAMETERS
     * frame :              [IN] frame pointer.
     * x :                  [IN] X offset of start position.
     * y :                  [IN] Y offset of start position.
     * text :               [IN] Text encoded using ucs2 format.
     * baseline :           [IN] The baseline to be aligned to.
     * RETURNS
     *   VM_RESULT refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_text_by_baseline(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMWSTR text,
            VMINT baseline);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_draw_truncated_text
     * DESCRIPTION
     * Draws the texts at the given position within the frame. Will be truncated if necessary with
     * configured truncate symbol.
     * PARAMETERS
     *  frame :             [IN] Frame pointer
     *  x :                 [IN] Offset X
     *  y :                 [IN] Offset Y
     *  width :             [IN] Draws area width, if the text is longer than the
     *                           assigned width, the end of the text will be truncated and ended with
     *                           truncated symbol.
     *  text :              [IN] Text to show
     *  truncated_symbol :  [IN] Truncated symbol, for example,"..."
     *  bordered :          [IN] Show border or not.
     * RETURNS
     *  VM_RESULT refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_draw_truncated_text(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width,
            VMWSTR text, VMWSTR truncated_symbol, VMINT bordered);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_set_font_size
    * DESCRIPTION
    * Set the global font size. You can use the VM_GRAPHIC_FONT_SIZE enum. If the platform
    * supports vector font you will be able to set the real font size in pixel, but it should
    * be bigger than 2 for the value that is occupied by VM_GRAPHIC_FONT_SIZE enum.
    * PARAMETERS
    * Size : [IN] font size
    * RETURNS
    *  VM_RESULT refer to VM_GRAPHIC_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_set_font_size(VMINT size);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_set_font_style
    * DESCRIPTION
    * Set the font style. Only valid for vector fonts.
    * PARAMETERS
    * bold :        [IN] Set style to bold, value:TRUE or FALSE
    * italic :      [IN] Set style to italic, value:TRUE or FALSE
    * underline:    [IN] Set style to underline, value:TRUE or FALSE
    * RETURNS
    *  VM_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_set_font_style(VMINT bold, VMINT italic, VMINT underline);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_is_vector_font_supported
    * DESCRIPTION
    * Query the current platform if it supports vector fonts.
    * PARAMETERS
    * void
    * RETURNS
    *  If they are support it return VM_TRUE, otherwise returns VM_FALSE
    *****************************************************************************/
    VMBOOL vm_graphic_is_vector_font_supported(void);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_get_font_pool_size
    * DESCRIPTION
    *  Get the memory size needed for the font pool. The user can allocate memory with desired
    *  size, and set it as font pool size by calling vm_graphic_init_font_pool();
    * PARAMETERS
    * max_external_font_size :   [IN] Bytes, the actual font size when font is loaded in RAM, it can be get by tool.
    * max_external_font_number : [IN] Max numbers of fonts you want to install.
    * max_cache_size :           [IN] Bytes, normally 64KB ~ 128KB, need more cache if have simplified / Traditional Chinese(128KB ~ 256KB).
    * pool_size :                [OUT] Bytes, total pool size
    * RETURNS
    *   VM_RESULT refer to VM_GRAPHIC_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_get_font_pool_size(VMUINT32 max_external_font_size, VMUINT32 max_external_font_number,
                                            VMUINT32 max_cache_size, VMUINT32* pool_size);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_init_font_pool
    * DESCRIPTION
    *  Initialize the font pool with outside memory.
    * PARAMETERS
    * pool :    [IN] The pointer to the pool memory allocated by the application in bytes
    * size :    [IN] The size of pool memory in bytes
    * RETURNS
    *   VM_RESULT refer to VM_GRAPHIC_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_init_font_pool(VMUINT8* pool, VMUINT32 pool_size);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_set_font
    * DESCRIPTION
    *  Set app's own fonts, afterward, engine will try to find the font for a character from these font files one by one,
    *  the font with smaller index in the font_file_paths array will have higher priority.
    *  If there is no related font in thest font files for a character, the system font will be used.
    *  Please call this API after VM_MSG_PAINT event.
    *  Set fonts clears the internal cache of the text rendering engine,
    *  therefore performance-wise it is better to prevent Reset/Set fonts frequently.
    * PARAMETERS
    * font_file_paths :   [IN] array of font paths, fullpath of the font data file,
    * for example, L"E:\\MTKYH2312F.ttf"
    * number_of_fonts :   [IN] number of fonts
    * RETURNS
    *  VM_RESULT refer to VM_GRAPHIC_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_set_font(VMWSTR font_file_paths[], VMINT number_of_fonts);

    /******************************************************************************
    * FUNCTION
    *  vm_graphic_reset_font
    * DESCRIPTION
    *  Reset app's own fonts, afterward, the texts will be drawn using system font
    * PARAMETERS
    *  void
    * RETURNS
    *   refer to VM_GRAPHIC_RESULT
    *****************************************************************************/
    VM_RESULT vm_graphic_reset_font(void);


    /******************************************************************************
     * FUNCTION
     *  vm_graphic_get_char_height_common_language
     * DESCRIPTION
     *  get current font height only considering English and Chinese, for dot
     *  character, it is precise. for vector font,it is usually higher than
     *  single character, it is the text height.
     * PARAMETERS
     *  void
     * RETURNS
     *  the height of the font
     *****************************************************************************/
    VMINT vm_graphic_get_char_height_common_language(void);

    /*****************************************************************************
     * FUNCTION
     *  vm_font_get_char_height_all_language
     * DESCRIPTION
     * Get maximum char height regardless of language for the font size
     * PARAMETERS
     *  void
     * RETURNS
     *  return char height
     *****************************************************************************/
    VMINT vm_graphic_get_char_height_all_language(void);

    /*****************************************************************************
     * FUNCTION
     *  vm_font_get_char_baseline_all_language
     * DESCRIPTION
     * Get maximum char baseline in pixel regardless of language for the font size
     * PARAMETERS
     *  size   [IN] font size
     * RETURNS
     *  return char baseline
     *****************************************************************************/
    VMINT vm_graphic_get_char_baseline_all_language(VMINT size);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_character_width
     * DESCRIPTION
     * Get the width in pixel of a character
     * PARAMETERS
     *  character :         [IN] character in ucs2 format
     * RETURNS
     *  >= 0 means the width of the character, <0 error code
     *****************************************************************************/
    VMINT vm_graphic_get_character_width(VMWCHAR character);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_character_height
     * DESCRIPTION
     * Get the height in pixel of a character
     * PARAMETERS
     *  c :         [IN] character in ucs2 format
     * RETURNS
     *  >= 0 means the height of the character, <0 error code
     *****************************************************************************/
    VMINT vm_graphic_get_character_height(VMWCHAR character);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_measure_character
     * DESCRIPTION
     *  get the width and height of the appointed character. Unit:pixel
     * PARAMETERS
     *  c	 :                  [IN]the value of character encoded in ucs2 format
     *  width :                 [OUT]character width
     *  height :                [OUT]character height
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_measure_character(VMWCHAR character, VMINT* width, VMINT* height);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_get_character_info
     * DESCRIPTION
     *  get the information of the appointed character. Unit:pixel
     * PARAMETERS
     *  character:                  [IN]the value of character encoded in ucs2 format
     *  character_info :             [OUT]character information
     * RETURNS
     *   refer to VM_GRAPHIC_RESULT
     *****************************************************************************/
    VM_RESULT vm_graphic_get_character_info(VMWCHAR character, vm_graphic_character_info_t* character_info);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_get_text_width
     * DESCRIPTION
     *  get the width of a text in pixels
     * PARAMETERS
     *  text :     [IN] text encoded in ucs2 format
     * RETURNS
     *  >=0 the width of text, <0 error code
     *****************************************************************************/
    VMINT vm_graphic_get_text_width(VMWSTR text);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_get_text_height
     * DESCRIPTION
     *  get the height of a text in pixels
     * PARAMETERS
     *  text :     [IN] text encoded in ucs2 format
     * RETURNS
     *  >=0 the height of text, <0 error code
     *****************************************************************************/
    VMINT vm_graphic_get_text_height(VMWSTR text);

    /******************************************************************************
     * FUNCTION
     *  vm_graphic_get_text_baseline
     * DESCRIPTION
     *  get the baseline of a text in pixels
     * PARAMETERS
     *  text :     [IN] text encoded in ucs2 format
     * RETURNS
     *  >=0 the baseline of text, <0 error code
     *****************************************************************************/
    VMINT vm_graphic_get_text_baseline(VMWSTR text);

    /*****************************************************************************
     * FUNCTION
     *  vm_graphic_get_text_width_height
     * DESCRIPTION
     * PARAMETERS
     *  text   [IN] text encoded in ucs2 format
     *  gap : [IN] gap between each character.
     *  width : [OUT] width
     *  height : [OUT] height
     *  max_width : [IN] the draw area for the text
     *  check_line_break : [IN] check line break or not
     *  check_complete_word : [IN] check complete word or not
     * RETURNS
    *  how many character can draw in given width
     *****************************************************************************/
    VMINT vm_graphic_get_text_width_height(VMWSTR text, VMINT gap, VMINT* width, VMINT* height,
                                           VMINT max_width, VMUINT8 check_line_break, VMUINT8 check_complete_word);


#ifdef _cplusplus
}
#endif
#endif /* VMGRAPHIC_FONT_SDK_H */

