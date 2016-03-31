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

#ifndef VMGRAPHIC_SDK_H
#define VMGRAPHIC_SDK_H

#ifdef __cplusplus
extern "C" {
#endif

#include "vmtype.h"


/* Graphic rotate type */
typedef enum
{
    VM_GRAPHIC_ROTATE_NORMAL = 0,           /* Rotate 0 degree */
    VM_GRAPHIC_ROTATE_90 = 0x01,            /* Rotate 90 degree */
    VM_GRAPHIC_ROTATE_180 = 0x02,           /* Rotate 180 degree */
    VM_GRAPHIC_ROTATE_270 = 0x03,           /* Rotate 270 degree */
    VM_GRAPHIC_MIRROR = 0x80,               /* Mirror */
    VM_GRAPHIC_MIRROR_ROTATE_90 = 0x81,     /* Mirror 90 degree */
    VM_GRAPHIC_MIRROR_ROTATE_180 = 0x82,    /* Mirror 180 degree */
    VM_GRAPHIC_MIRROR_ROTATE_270 = 0x83,    /* Mirror 270 degree */
    VM_GRAPHIC_ROTATE_TYPE_MAX = 0x7FFFFFFF /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_GRAPHIC_ROTATE_TYPE;

/* Gradient style */
typedef enum
{
    VM_GRAPHIC_GRADIENT_HORIZONTAL = 0,             /* Horizontal */
    VM_GRAPHIC_GRADIENT_VERTICAL,                   /* Vertical */
    VM_GRAPHIC_GRADIENT_DIA,                        /* Lean 45 degree */
    VM_GRAPHIC_GRADIENT_DIA_INV,                    /* De-lean 45 degree */
    VM_GRAPHIC_GRADIENT_FLIP,                       /* From the middle to the edges */
    VM_GRAPHIC_GRADIENT_STYLE_MAX = 0x7FFFFFFF      /* Ensure the compiler treat this enum as 32-bit integer. */
} VM_GRAPHIC_GRADIENT_STYLE;

/* Point structure */
typedef struct
{
    VMINT16 x;                              /* X offset */
    VMINT16 y;                              /* Y offset */
} vm_graphic_point_t;

/* Color struct in argb format */
typedef struct
{
    VMUINT8 a;                              /* Alpha */
    VMUINT8 r;                              /* Red   */
    VMUINT8 g;                              /* Green */
    VMUINT8 b;                              /* Blue  */
} vm_graphic_color_argb_t;

/******************************************************************************
 * FUNCTION
 *  vm_graphic_set_clip
 * DESCRIPTION
 *  Sets the clip, it will affect all the graphic function calls.
 *  The coordinates are frame-based.
 * PARAMETERS
 *  x0 : [IN] X coordinate of upper left corner
 *  y0 : [IN] Y coordinate of upper left corner
 *  x1 : [IN] X coordinate of lower right corner
 *  y1 : [IN] Y coordinate of lower right corner
 *****************************************************************************/
void vm_graphic_set_clip(VMINT x0, VMINT y0, VMINT x1, VMINT y1);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_reset_clip, it will affect all the graphic function calls.
 * DESCRIPTION
 *  Reset the clip
 *****************************************************************************/
void vm_graphic_reset_clip(void);




/* Graphic results */
typedef enum
{
    VM_GRAPHIC_SUCCESS = 0,             /* graphic operation success */
    VM_GRAPHIC_FAIL = -1,               /* graphic operation fail */
    VM_GRAPHIC_WRONG_PARAMETER = -2,    /* Wrong parameter */
    VM_GRAPHIC_RESULT_MAX = 0x7FFFFFFF
} VM_GRAPHIC_RESULT;

/* Graphic color formats */
typedef enum
{
    VM_GRAPHIC_COLOR_FORMAT_16_BIT = 0,      /* 16-bit, RGB 565 */
    VM_GRAPHIC_COLOR_FORMAT_32_BIT = 2,      /* 32-bit, B,G,R,A     8,8,8,8 */
    VM_GRAPHIC_COLOR_FORMAT_32_PARGB = 3,    /* 32-bit, PB,PG,PR,A  8,8,8,8 */
    VM_GRAPHIC_COLOR_FORMAT_MAX = 0x7F
} VM_GRAPHIC_COLOR_FORMAT;

/* Frame struct */
typedef struct
{
    VMINT width;                                /* Frame width */
    VMINT height;	                            /* Frame height */
    VM_GRAPHIC_COLOR_FORMAT color_format;       /* Color format */
    VMINT buffer_length;                        /* Frame buffer length */
    VMUINT8* buffer;                            /* Frame buffer */
} vm_graphic_frame_t;

/* Transform source key edge filters */
typedef enum
{
    VM_GRAPHIC_TRANSFORM_SOURCE_KEY_EDGE_FILTER_DULPLICATE,             /* Duplicate */
    VM_GRAPHIC_TRANSFORM_SOURCE_KEY_EDGE_FILTER_USER_SPECIFIED_COLOR,   /* User specified color */
    VM_GRAPHIC_TRANSFORM_SOURCE_KEY_EDGE_FILTER_MAX = 0x7FFFFFFF
} VM_GRAPHIC_TRANSFORM_SOURCE_KEY_EDGE_FILTER;

/* Transform sample mode */
typedef enum
{
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_TRUNCATED,                                      /* Truncated */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_NEAREST,                                        /* Nearest */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_BILINEAR_WITHOUT_EDGE_FILTER,                   /* Bilinear without edge filter */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_BILINEAR_WITH_EDGE_FILTER_DULPLICATE,           /* Bilinear with edge filter duplicate */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_BILINEAR_WITH_EDGE_FILTER_DESTINATION_COLOR,    /* Bilinear with edge filter destination color */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_BILINEAR_WITH_EDGE_FILTER_USER_SPECIFIED_COLOR, /* Bilinear with edge filter user specified color */
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE_MAX = 0x7FFFFFFF
} VM_GRAPHIC_TRANSFORM_SAMPLE_MODE;

/* Transform flags */
typedef enum
{
    VM_GRAPHIC_TRANSFORM_FLAG_USE_INVERSE_MATRIX          =  (1<<0),       /* Use inverse matrix */
    VM_GRAPHIC_TRANSFORM_FLAG_NONE_BLOCKING                =  (1<<1),      /* None blocking */
    VM_GRAPHIC_TRANSFORM_FLAG_USE_DIRECT_OUTPUT           =  (1<<2),       /* Use direct output */
    VM_GRAPHIC_TRANSFORM_FLAG_USE_RANDOM_ALGORITHM_DITHER =  (1<<3),       /* Use random algorithm dither */
    VM_GRAPHIC_TRANSFORM_FLAG_USE_FIXED_PATTERN_DITHER    =  (1<<4),       /* Use fixed pattern dither */
    VM_GRAPHIC_TRANSFORM_FLAG_DISABLE_SOURCE_KEY          =  (1<<5),       /* Disable source key */
    VM_GRAPHIC_TRANSFORM_FLAG_DISABLE_CACHE_SWITCH        =  (1<<6),       /* Disable cache switch */
    VM_GRAPHIC_TRANSFORM_FLAG_MAX = 0xFFFFFFFF
} VM_GRAPHIC_TRANSFORM_FLAG;

/******************************************************************************
 * FUNCTION
 *  vm_graphic_set_color
 * DESCRIPTION
 *  Sets the global color for graphic functions such as draw text or shapes
 * PARAMETERS
 *  color : [IN] the color value in RGBA
 * RETURNS
 * VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_set_color(vm_graphic_color_argb_t color);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_get_color
 * DESCRIPTION
 *  Get global color
 * PARAMETERS
 *  color : [OUT] The color value in RGBA
 * RETURNS
 *   VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_get_color(vm_graphic_color_argb_t* color);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_get_point_color
 * DESCRIPTION
 *  Gets the color of the appointed point in the frame
 * PARAMETERS
 *  frame: [IN] frame pointer
 *  color: [OUT] The color value in RGBA
 *  x	 : [IN] x offset of the point
 *  y	 : [IN] y offset of the point
 * RETURNS
 *  VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_get_point_color(vm_graphic_frame_t* frame, vm_graphic_color_argb_t* color, VMINT x, VMINT y);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_flatten_frame
 * DESCRIPTION
 *  Flatten frames to output_frame
 * PARAMETERS
 *  out_frame: [IN] Output frame, all the frames will be flattened to this frame.
 *  frames:    [IN] Frames to be flattened.
 *  positions: [IN] Positions in the out_frame of each frame.
 *  number:    [IN] The number of frame to be flattened.
 * RETURNS
 *  VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_flatten_frame(vm_graphic_frame_t* out_frame,
                                   const vm_graphic_frame_t* frames[],
                                   const vm_graphic_point_t positions[],
                                   VMINT number);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_blt_frame
 * DESCRIPTION
 *  Blot frames to LCD, this function only takes effect for MediaTek LCM.
 *  The frames don't have to be the same size as the LCM size, it will be blot
 *  from the corresponding positions. The color format of the frames should be
 *  same.
 * PARAMETERS
 *  frames :    [IN] Frames to be blotted.
 *  positions : [IN] Positions for each frame.
 *  number :    [IN] The number of frames, the maximum number is 4.
 * RETURNS
 *  VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_blt_frame(const vm_graphic_frame_t* frames[],
                               const vm_graphic_point_t positions[],
                               VMINT number);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_rotate_frame
 * DESCRIPTION
 *  Rotates a frame according to the rotate type, the frame is rotated in-place.
 * PARAMETERS
 *  out_frame :     [IN] Output frame.
 *  in_frame :      [IN] Input frame.
 *  rotate_type :   [IN] The rotation type.
 * RETURNS
 *   VM_RESULT refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_rotate_frame(
    vm_graphic_frame_t* out_frame,
    vm_graphic_frame_t* in_frame,
    VM_GRAPHIC_ROTATE_TYPE rotate_type);


/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_point
 * DESCRIPTION
 *  Draws a point at position(x,y) within the frame
 * PARAMETERS
 *  frame : [IN] Frame pointer
 *  x	 :  [IN] X of the point
 *  y	 :  [IN] Y of the point
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_point(vm_graphic_frame_t* frame, VMINT x, VMINT y);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_line
 * DESCRIPTION
 *  Draws a line from position(x0, y0) to position(x1, y1) within the frame
 * PARAMETERS
 *  frame : [IN] frame pointer
 *  x0	 :  [IN] X of the start point
 *  y0	 :  [IN] Y of the start point
 *  x1	 :  [IN] X of the end point
 *  y1	 :  [IN] Y of the end point
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_line( vm_graphic_frame_t* frame, VMINT x0, VMINT y0,
                           VMINT x1, VMINT y1);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_rectangle
 * DESCRIPTION
 *  Draws a rectangle at the given point within the frame.
 * PARAMETERS
 *  frame : [IN] Frame pointer
 *  x	 :  [IN] X offset of the start point
 *  y :     [IN] Y offset of the start point
 *  width : [IN] Width of the rectangle
 *  height :[IN] Height of the rectangle
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_rectangle(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width, VMINT height);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_solid_rectangle
 * DESCRIPTION
 *  Draws a solid rectangle at the given point within the frame.
 * PARAMETERS
 *  frame : [IN] Frame pointer
 *  x	 :  [IN] X offset of the start point
 *  y	 :  [IN] Y offset of the start point
 *  width : [IN] Width of the rectangle
 *  height: [IN] Height of the rectangle
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_solid_rectangle(vm_graphic_frame_t* frame,VMINT x,VMINT y,VMINT width,VMINT height);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_round_rectangle
 * DESCRIPTION
 *  Draws a rectangle with rounded corners at the given point within the frame.
 * PARAMETERS
 *  frame :         [IN] Frame pointer
 *  x	 :          [IN] X offset of the start point
 *  y :             [IN] Y offset of the start point
 *  width :         [IN] Width of the round rectangle
 *  height	 :      [IN] Height of the round rectangle
 * corner_width :   [IN] Radius of the rounded angle. The value must be less than half of the width,
 *                       if not, the function will have no effect.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_round_rectangle(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width, VMINT height,
                                     VMINT corner_width);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_solid_round_rectangle
 * DESCRIPTION
 *  Draws a solid rectangle with rounded corners at the given point within the frame.
 * PARAMETERS
 *  frame :     [IN] Frame pointer
 *  x	 :      [IN] X offset of the start point
 *  y	 :      [IN] Y offset of the start point
 *  width :     [IN] Width of the round rectangle
 *  height :    [IN] Height of the round rectangle
 * corner_width:[IN] Radius of the rounded angle, the value must less than half of width,
 *                   if not, the function will have no effect
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_solid_round_rectangle(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width, VMINT height,
        VMINT corner_width);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_ellipse
 * DESCRIPTION
 *  Draws an ellipse at the given point within the frame.
 * PARAMETERS
 *  frame :     [IN] Frame pointer.
 *  x	 :      [IN] X offset of the start point.
 *  y	 :      [IN] Y offset of the start point.
 *  width :     [IN] Width of the ellipse.
 *  height	 :  [IN] Height of the ellipse.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_ellipse(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width, VMINT height);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_solid_ellipse
 * DESCRIPTION
 *  Draws a solid ellipse at the given point within the frame.
 * PARAMETERS
 *  frame :     [IN] Frame pointer.
 *  x	 :      [IN] X offset of the start point.
 *  y	 :      [IN] Y offset of the start point.
 *  width :     [IN] Width of the ellipse.
 *  height	 :  [IN] Height of the ellipse.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_solid_ellipse(vm_graphic_frame_t* frame, VMINT x, VMINT y, VMINT width, VMINT height);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_polygon
 * DESCRIPTION
 *  Draws a polygon at the given point within the frame.
 * PARAMETERS
 *  frame :     [IN] Frame pointer.
 *  point :     [IN] An array of point coordinates.
 *  npoints :   [IN] Number of points.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_polygon(vm_graphic_frame_t* frame, const vm_graphic_point_t points[],VMINT number_of_points);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_solid_polygon
 * DESCRIPTION
 * Draws a solid polygon at the given point within the frame.
 * PARAMETERS
 *  frame :     [IN] Frame pointer.
 *  point :     [IN] An array of point coordinates.
 *  npoints :   [IN] Number of points.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_solid_polygon(vm_graphic_frame_t* frame, const vm_graphic_point_t points[],VMINT number_of_points);

/******************************************************************************
 * FUNCTION
 *  vm_graphic_draw_gradient_rectangle
 * DESCRIPTION
 * Draws a gradient rectangle at the given point within the frame.
 * PARAMETERS
 *  frame :         [IN] Frame pointer.
 *  x0 :            [IN] X offset of top left position.
 *  y0 :            [IN] Y offset of top left position.
 *  x1 :            [IN] X offset of bottom right position.
 *  y1 :            [IN] Y offset of bottom right position.
 * color_start :    [IN] Start color of gradient color.
 * color_end :      [IN] End color of gradient color.
 * gradient :       [IN] Gradient style.
 * RETURNS
 *  void
 *****************************************************************************/
void vm_graphic_draw_gradient_rectangle(vm_graphic_frame_t* frame, VMINT x0, VMINT y0, VMINT x1, VMINT y1,
                                        vm_graphic_color_argb_t color_start, vm_graphic_color_argb_t color_end,VM_GRAPHIC_GRADIENT_STYLE gradient);

/*****************************************************************************
 * FUNCTION
 *  vm_graphic_linear_transform
 * DESCRIPTION
 * Linear transformation
 * PARAMETERS
 *  out_frame                       [IN] Output frame
 *  in_frame                        [IN] Input frame
 *  matrix                          [IN] 3X3 row-major transform matrix
 *  src_rect_x                      [IN] Source rectangle top left position X
 *  src_rect_y                      [IN] Source rectangle top left position Y
 *  src_rect_w                      [IN] Source rectangle width
 *  src_rect_h                      [IN] Source rectangle height
 *  dst_rect_x                      [IN] Destination rectangle top left position X
 *  dst_rect_y                      [IN] Destination rectangle top left position Y
 *  dst_rect_w                      [IN] Destination rectangle width
 *  dst_rect_h                      [IN] Destination rectangle height
 *  edge_filter_mode                [IN] How to handle source key edge
 *  edge_filter_color               [IN] Color to use if edge_filter_mode is USER_SPEC_COLOR
 *  sample_mode                     [IN] How to handle buffer edge
 *  flag                            [IN] Refer to VM_GRAPHIC_TRANSFORM_FLAG
 * RETURNS
 *   refer to VM_GRAPHIC_RESULT
 *****************************************************************************/
VM_RESULT vm_graphic_linear_transform(
    vm_graphic_frame_t* out_frame,
    vm_graphic_frame_t* in_frame,
    VMFLOAT* matrix,
    VMINT32 source_rectangle_x,
    VMINT32 source_rectangle_y,
    VMINT32 source_rectangle_width,
    VMINT32 source_rectangle_height,
    VMINT32 destination_rectangle_x,
    VMINT32 destination_rectangle_y,
    VMINT32 destination_rectangle_width,
    VMINT32 destination_rectangle_height,
    VM_GRAPHIC_TRANSFORM_SOURCE_KEY_EDGE_FILTER edge_filter_mode,
    vm_graphic_color_argb_t edge_filter_color,
    VM_GRAPHIC_TRANSFORM_SAMPLE_MODE sample_mode,
    VMUINT32 flag);



#ifdef __cplusplus
}
#endif

#endif
