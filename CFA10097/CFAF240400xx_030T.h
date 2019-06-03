#ifndef __CFAF240400xx_030T_H__
#define __CFAF240400xx_030T_H__
//============================================================================
//
// Display definitions for:
//   CFA10097 with FT810 / FT811
//
// Displays:
//   CFAF240400C0-030SC (Cap Touch w/bright backlight)
//   https://www.crystalfontz.com/product/cfaf240400c0030sc
//   CFAF240400E0-030TN (Non-Touch)
//   https://www.crystalfontz.com/product/cfaf240400e0030tn
// Controller:
//   https://www.crystalfontz.com/controllers/Ilitek/ILI9327/
//
//
// 2018-11-21 Crystalfontz America, Inc. / Brent A. Crosby
//===========================================================================
//This is free and unencumbered software released into the public domain.
//
//Anyone is free to copy, modify, publish, use, compile, sell, or
//distribute this software, either in source code form or as a compiled
//binary, for any purpose, commercial or non-commercial, and by any
//means.
//
//In jurisdictions that recognize copyright laws, the author or authors
//of this software dedicate any and all copyright interest in the
//software to the public domain. We make this dedication for the benefit
//of the public at large and to the detriment of our heirs and
//successors. We intend this dedication to be an overt act of
//relinquishment in perpetuity of all present and future rights to this
//software under copyright law.
//
//THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
//EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
//MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
//IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
//OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
//ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
//OTHER DEALINGS IN THE SOFTWARE.
//
//For more information, please refer to <http://unlicense.org/>
//============================================================================
// Define RGB output pins order, determined by PCB layout
#define LCD_SWIZZLE      (0x2)
// Define active edge of PCLK. Observed by scope:
//  0: Data is put out coincident with falling edge of the clock.
//     Rising edge of the clock is in the middle of the data.
//  1: Data is put out coincident with rising edge of the clock.
//     Falling edge of the clock is in the middle of the data. 
// (ILI9327, P190)
#define LCD_PCLKPOL      (0)
// LCD drive strength: 0=5mA, 1=10mA
#define LCD_DRIVE_10MA   (0)
// Spread Spectrum on RGB signals. Probably not a good idea at higher
// PCLK frequencies.
#define LCD_PCLK_CSPREAD (0)
//This is a 24-bit display, so no need to dither.
#define LCD_DITHER       (0)
// Pixel clock divisor (based on 60MHz internal clock)
//   0 = disable
//   1 = 60MHz
//   2 = 30MHz
//   3 = 20MHz
//   etc
// Our target is 12MHz, (div of 5).
#define LCD_PCLK         (5)
//----------------------------------------------------------------------------
// Frame_Rate = 30MHz / (LCD_VCYCLE*LCD_HCYCLE)
//            = 30MHz / (863*511) = 68Hz or 14.7mS
//----------------------------------------------------------------------------
// Horizontal timing (typical values, ILI9327, P33)
// Target 60Hz frame rate, using the largest possible line time in order to
// maximize the time that the FT8xx has to process each line.
#define HPX   (240) // Horizontal Pixel Width
#define HSW   (10)  // Horizontal Sync Width
#define HBP   (20)  // Horizontal Back Porch
#define HFP   (10)  // Horizontal Front Porch
#define HPP   (209) // Horizontal Pixel Padding (FT8xx min is 1)
// Define the constants needed by the FT8xx based on the timing
// Active width of LCD display
#define LCD_WIDTH   (HPX)
// Start of horizontal sync pulse
#define LCD_HSYNC0  (HFP)
// End of horizontal sync pulse
#define LCD_HSYNC1  (HFP+HSW)
// Start of active line
#define LCD_HOFFSET (HFP+HSW+HBP)
// Total number of clocks per line
#define LCD_HCYCLE  (HPX+HFP+HSW+HBP+HPP)
//----------------------------------------------------------------------------
// Vertical timing (typical values, ILI9327, P33)
#define VLH   (400) // Vertical Line Height
#define VS    (2)   // Vertical Sync (in lines)
#define VBP   (2)   // Vertical Back Porch
#define VFP   (4)   // Vertical Front Porch
#define VLP   (1)   // Vertical Line Padding (FT8xx min is 1)
// Define the constants needed by the FT8xx based on the timing
// Active height of LCD display
#define LCD_HEIGHT  (VLH)
// Start of vertical sync pulse
#define LCD_VSYNC0  (VFP)
// End of vertical sync pulse
#define LCD_VSYNC1  (VFP+VS)
// Start of active screen
#define LCD_VOFFSET (VFP+VS+VBP)
// Total number of lines per screen
#define LCD_VCYCLE  (VLH+VFP+VS+VBP+VLP)
//============================================================================
#endif // __CFAF240400xx_030T_H__
