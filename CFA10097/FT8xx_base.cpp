//============================================================================
//
// Low-Level routines for FT8xx accelerators.
//
// This is a simplified / refactored version of the code in FTDI's AN_275:
//
//  http://brtchip.com/wp-content/uploads/Support/Documentation/Application_Notes/ICs/EVE/AN_275_FT800_Example_with_Arduino.pdf
//
// I have added support for the FT81x series.
//
// The write offset into the write buffer is passed into and back from
// functions rather than being a global.
//
// In the spirit of AN_275:
//
//   An “abstraction layer” concept was explicitly avoided in this
//   example. Rather, direct use of the Arduino libraries demonstrates
//   the simplicity of sending and receiving data through the FT800
//   while producing a graphic output.
//
// My main goal here is to be transparent about what is really happening
// from the high to lowest levels, without obfuscation, while still
// at least giving a nod to good programming practices.
//
// Plus, you probably don't have RAM and flash for all those fancy
// programming layers.
//
// A nod to Rudolf R and company over at
//   https://www.mikrocontroller.net/topic/395608
// for help understanding that the FT81x needs a 24-bit (full color)
// PNG, not an 8-bit paletized PNG.
//
//  2018 - 11 - 21 Brent A. Crosby / Crystalfontz
//---------------------------------------------------------------------------
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
// Adapted from:
// FTDIChip AN_275 FT800 with Arduino - Version 1.0
//
// Copyright (c) Future Technology Devices International
//
// THIS SOFTWARE IS PROVIDED BY FUTURE TECHNOLOGY DEVICES INTERNATIONAL
// LIMITED "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
// PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL FUTURE TECHNOLOGY
// DEVICES INTERNATIONAL LIMITED BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES LOSS OF USE,
// DATA, OR PROFITS OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
// OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
// EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// This code is provided as an example only and is not guaranteed by
// FTDI/BridgeTek. FTDI/BridgeTek accept no responsibility for any issues
// resulting from its use. By using this code, the developer of the final
// application incorporating any parts of this sample project agrees to take
// full responsible for ensuring its safe and correct operation and for any
// consequences resulting from its use.
//===========================================================================
#include <Arduino.h>
#include <SPI.h>
#include <stdarg.h>

// Definitions for our display.
#include "CFA10097_defines.h"
#include "CFAF240400xx_030T.h"

// The FT80x registers are different compared to the FT81x. Choose the FT81x.
#define FT_81X_ENABLE
#include "FT8xx_defines.h"
#include "FT8xx_base.h"
#include "FT8xx_draw.h"
//============================================================================
void _FT8xx_Select_and_Address(uint32_t Address, uint8_t Operation)
  {
  //Select the FT8xx
  CLR_FTDI_CS_NOT;
  // Send Operation plus high address byte
  SPI.transfer((uint8_t)(Address >> 16) | Operation);
  // Send middle address byte
  SPI.transfer((uint8_t)(Address >> 8));
  // Send low address byte
  SPI.transfer((uint8_t)(Address));
  }
//============================================================================
void _FT8xx_send_32(uint32_t Data)
  {
  // Send data low byte
  SPI.transfer((uint8_t)(Data));
  // Send data mid-low byte
  SPI.transfer((uint8_t)(Data >> 8));
  // Send data mid-high byte
  SPI.transfer((uint8_t)(Data >> 16));
  // Send data high byte
  SPI.transfer((uint8_t)(Data >> 24));
  }
//============================================================================
void FT8xx_REG_Write_8(uint32_t REG_Address, uint8_t ftData8)
  {
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_WRITE);
  // Send data byte
  SPI.transfer(ftData8);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  }
//============================================================================
void FT8xx_REG_Write_16(uint32_t REG_Address, uint16_t ftData16)
  {
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_WRITE);
  // Send data low byte
  SPI.transfer((uint8_t)(ftData16));
  // Send data high byte
  SPI.transfer((uint8_t)(ftData16 >> 8));
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  }
//============================================================================
void FT8xx_REG_Write_32(uint32_t REG_Address, uint32_t ftData32)
  {
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_WRITE);
  //Send the uint32_t ftData32
  _FT8xx_send_32(ftData32);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  }
//----------------------------------------------------------------------------
uint16_t FT8xx_Cmd_Dat_0(uint16_t FWol,
                         uint32_t command)
  {
  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);
  //Send the uint32_t data
  _FT8xx_send_32(command);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  //Increment address offset modulo 4096 and return it
  return((FWol+4)&0xFFF);
  }
//----------------------------------------------------------------------------
uint16_t FT8xx_Cmd_Dat_1(uint16_t FWol,
                         uint32_t command,uint32_t data0)
  {
  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);
  //Send the uint32_t data
  _FT8xx_send_32(command);
  //Send the first uint32_t data
  _FT8xx_send_32(data0);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  //Increment address offset modulo 4096 and return it
  return((FWol+8)&0xFFF);
  }
//----------------------------------------------------------------------------
uint16_t FT8xx_Cmd_Dat_2(uint16_t FWol,
                         uint32_t command,uint32_t data0, uint32_t data1)
  {
  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);
  //Send the uint32_t data
  _FT8xx_send_32(command);
  //Send the first uint32_t data
  _FT8xx_send_32(data0);
  //Send the second uint32_t data
  _FT8xx_send_32(data1);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  //Increment address offset modulo 4096 and return it
  return((FWol+12)&0xFFF);
  }
//----------------------------------------------------------------------------
uint16_t FT8xx_Cmd_Dat_3(uint16_t FWol,
                         uint32_t command,
                         uint32_t data0,
                         uint32_t data1,
                         uint32_t data2)
  {
  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);
  //Send the uint32_t data
  _FT8xx_send_32(command);
  //Send the first uint32_t data
  _FT8xx_send_32(data0);
  //Send the second uint32_t data
  _FT8xx_send_32(data1);
  //Send the third uint32_t data
  _FT8xx_send_32(data2);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  //Increment address offset modulo 4096 and return it
  return((FWol+16)&0xFFF);
  }
//============================================================================
uint8_t FT8xx_REG_Read_8(uint32_t REG_Address)
  {
  uint8_t
    ftData8;
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_READ);
  // Send dummy byte
  SPI.transfer(0);
  // Send another dummy, but this time the FT8xx will reply with the goods.
  ftData8 = SPI.transfer(0);
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  // Return uint8_t read
  return(ftData8);
  }
//============================================================================
uint16_t FT8xx_REG_Read_16(uint32_t REG_Address)
  {
  uint16_t
    ftData16;
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_READ);
  // Send dummy byte
  SPI.transfer(0);
  // Send more dummies, but now the FT8xx will reply with the goods.
  // Read low byte
  ftData16 = SPI.transfer(0);
  // Read high byte
  ftData16 |= (uint16_t)SPI.transfer(0) << 8;
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  // Return uint16_t read
  return(ftData16);
  }
//============================================================================
uint32_t FT8xx_REG_Read_32(uint32_t REG_Address)
  {
  uint32_t
    ftData32;
  //Select the FT8xx and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(REG_Address, MEM_READ);
  // Send dummy byte
  SPI.transfer(0);
  // Send more dummies, but now the FT8xx will reply with the goods.
  // Read low byte
  ftData32 = (uint32_t)SPI.transfer(0);
  // Read mid-low byte
  ftData32 |= (uint32_t)SPI.transfer(0) << 8;
  // Read mid-high byte
  ftData32 |= (uint32_t)SPI.transfer(0) << 16;
  // Read high byte
  ftData32 |= (uint32_t)SPI.transfer(0) << 24;
  //De-select the FT8xx
  SET_FTDI_CS_NOT;
  // Return uint32_t read
  return(ftData32);
  }
//============================================================================
// Wait for graphics processor to complete executing the current command
// list. This happens when REG_CMD_READ matches REG_CMD_WRITE, indicating
// that all commands have been executed.  We have a local copy of
// REG_CMD_WRITE in SW_write_offset.
#if 0
//Simple, fast, no checking.
#define Wait_for_FT8xx_Execution_Complete(SW_write_offset) while(FT8xx_REG_Read_16(REG_CMD_READ) != SW_write_offset)
#else
//Debugging
void Wait_for_FT8xx_Execution_Complete(uint16_t SW_write_offset)
  {
#if 0
  uint32_t
    vector;
  vector=0x0FAA55F0;
#endif
  uint16_t
    read_address;
  uint16_t
    write_address;

  uint16_t
    reported_read_address;
  uint16_t
    reported_write_address;
  reported_read_address=0;
  reported_write_address=0;

  do
    {
    read_address=FT8xx_REG_Read_16(REG_CMD_READ);
    write_address=FT8xx_REG_Read_16(REG_CMD_WRITE);
    if((read_address&0xF003)||(write_address&0xF003)||(SW_write_offset&0xF003)||(write_address!=SW_write_offset))
      {
      if((reported_read_address!=read_address)||(reported_write_address!=write_address))
        {
        if(write_address!=SW_write_offset)
          SerPrintFF(F("Write Mismatch: HDW_R=(%5u,0x%04X) HDW_W=(%5u,0x%04X) != SW_W=(%5u,0x%04X)\n"),
                       read_address,read_address,
                       write_address,write_address,
                       SW_write_offset,SW_write_offset);
        if((read_address&0xF003)||(write_address&0xF003)||(SW_write_offset&0xF003))
          SerPrintFF(F("DWORD Alignment: HDW_R=(%5u,0x%04X) HDW_W=(%5u,0x%04X) SW_W=(%5u,0x%04X)\n"),
                       read_address,read_address,
                       write_address,write_address,
                       SW_write_offset,SW_write_offset);
        reported_read_address=read_address;
        reported_write_address=write_address;
        }
      }
#if 0
    //Write and read REG_MACRO_0 to debug SPI
    uint32_t
      readback;
    SET_DEBUG_LED;
    FT8xx_REG_Write_32(REG_MACRO_0, vector);
    CLR_DEBUG_LED;
    readback=FT8xx_REG_Read_32(REG_MACRO_0);
    if(readback!=vector)
      SerPrintFF(F("wrote: 0x%08lX read: 0x%08lX\n"),vector,readback);
    vector^=0xFFFFFFFF;
#endif
    }while(read_address!=SW_write_offset);
  }
#endif
//============================================================================
uint16_t Get_Free_CMD_Space(uint16_t FWol)
  {
  // (4K - 4) - (write-read)
  return((4096-4)-((FWol-FT8xx_REG_Read_16(REG_CMD_READ))&0x0FFF));
  }
//============================================================================
//from BRT_AN_014 FT81X Simple PIC Library Examples:
// An optional step then follows to allow the end of the inflated data to be
// determined, which will be useful when loading data into subsequent RAM_G
// locations to make efficient use of the memory. Whilst the commands above
// define that the data will begin at 0, the end address is not known as the
// inflated data will be larger than the compressed data written to the
// CMD_INFLATE command. The CMD_GETPTR can be used to check the ending
// address. This command actually returns its value via the co-processor
// FIFO itself. The command (4 bytes) is sent with a dummy (4-byte) value as
// a parameter and so occupies two 32-bit locations in the command FIFO. On
// completion, the co-processor replaces the dummy value with the ending
// address. The application must then perform a read of this location (which
// can be obtained by checking the current REG_CMD_WRITE pointer and
// subtracting 4 taking account of any possible rollover at (RAM_CMD+0).
//----------------------------------------------------------------------------
uint16_t Get_RAM_G_Pointer_After_INFLATE(uint16_t FWol,
                                         uint32_t *RAM_G_First_Available)
  {
  //Make sure that the chip is caught up.
  Wait_for_FT8xx_Execution_Complete(FWol);
  //Tell the chip to get the first free location in RAM_G
  FWol=FT8xx_Cmd_Dat_1(FWol,
                       CMD_GETPTR,0);
  // Update the ring buffer pointer so the graphics processor starts executing
  FT8xx_REG_Write_16(REG_CMD_WRITE,FWol);
  //Wait for the chip to catch up.
  Wait_for_FT8xx_Execution_Complete(FWol);

  //We know that the answer is 4 addresses lower than FWol
  //Read the value and passs it back to the caller know what we found.
  *RAM_G_First_Available=FT8xx_REG_Read_32(RAM_CMD+((FWol-4) & 0x0FFF));

  return(FWol);
  }
//============================================================================
uint16_t Get_RAM_G_Properties_After_LOADIMAGE(uint16_t FWol,
                                              uint32_t *RAM_G_First_Available,
                                              uint32_t *Width,
                                              uint32_t *Height)
  {
  //Make sure that the chip is caught up.
  Wait_for_FT8xx_Execution_Complete(FWol);
  // Tell the chip to get the first free location in RAM_G
  FWol=FT8xx_Cmd_Dat_3(FWol,
                       CMD_GETPROPS,0,0,0);
  // Update the ring buffer pointer so the graphics processor starts executing
  FT8xx_REG_Write_16(REG_CMD_WRITE,FWol);
  //Wait for the chip to catch up.
  Wait_for_FT8xx_Execution_Complete(FWol);

  //We know that the height is 4 addresses lower than FWol
  //Read the value and passs it back to the caller know what we found.
  *Height=FT8xx_REG_Read_32(RAM_CMD+((FWol-4) & 0x0FFF));

  //We know that the width is 8 addresses lower than FWol
  //Read the value and passs it back to the caller know what we found.
  *Width=FT8xx_REG_Read_32(RAM_CMD+((FWol-8) & 0x0FFF));

  //We know that the answer is 12 addresses lower than FWol
  //Read the value and passs it back to the caller know what we found.
  *RAM_G_First_Available=FT8xx_REG_Read_32(RAM_CMD+((FWol-12) & 0x0FFF));
  return(FWol);
  }
//============================================================================
#if (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE==CAP_DEV_GT911)
//Used to send cap-touch init code to FT8xx from progmem
//Magic binary data from "AN_336 FT8xx - Selecting an LCD Display"
#define GOODIX_GT911_INIT_DATA_LENGTH (1216)
const uint8_t Goodix_GT911_Init_Data[GOODIX_GT911_INIT_DATA_LENGTH] PROGMEM = {
  0x1A,0xFF,0xFF,0xFF,0x20,0x20,0x30,0x00,0x04,0x00,0x00,0x00,0x02,0x00,0x00,0x00,
  0x22,0xFF,0xFF,0xFF,0x00,0xB0,0x30,0x00,0x78,0xDA,0xED,0x54,0xDD,0x6F,0x54,0x45,
  0x14,0x3F,0x33,0xB3,0x5D,0xA0,0x94,0x65,0x6F,0x4C,0x05,0x2C,0x8D,0x7B,0x6F,0xA1,
  0x0B,0xDB,0x9A,0x10,0x09,0x10,0x11,0xE5,0x9C,0x4B,0x1A,0x0B,0x0D,0x15,0xE3,0x03,
  0x10,0xFC,0xB8,0xB3,0x2D,0xDB,0x8F,0x2D,0x29,0x7D,0x90,0x48,0x43,0x64,0x96,0x47,
  0xBD,0x71,0x12,0x24,0x11,0xA5,0x64,0xA5,0xC6,0x10,0x20,0x11,0x95,0xC4,0xF0,0x80,
  0xA1,0x10,0xA4,0x26,0x36,0xF0,0x00,0xD1,0x48,0x82,0x0F,0x26,0x7D,0x30,0x42,0x52,
  0x1E,0x4C,0x13,0x1F,0xAC,0x67,0x2E,0x8B,0x18,0xFF,0x04,0xE3,0x9D,0xCC,0x9C,0x33,
  0x73,0x66,0xCE,0xE7,0xEF,0xDC,0x05,0xAA,0x5E,0x81,0x89,0x4B,0xC2,0xD8,0x62,0x5E,
  0x67,0x75,0x73,0x79,0x4C,0x83,0xB1,0x7D,0x59,0x7D,0x52,0x7B,0x3C,0xF3,0x3A,0x8E,
  0xF2,0xCC,0xB9,0xF3,0xBC,0x76,0x9C,0xE3,0x9B,0xCB,0xEE,0xEE,0xC3,0xFB,0xCD,0xE5,
  0x47,0x5C,0x1C,0xA9,0xBE,0xB8,0x54,0x8F,0x71,0x89,0x35,0xF4,0x67,0xB5,0xED,0x57,
  0xFD,0x71,0x89,0xE9,0x30,0x0C,0xC6,0xA5,0xB5,0x68,0x8B,0x19,0x54,0xFD,0x9B,0x72,
  0x4A,0xBF,0x00,0x36,0x8A,0xA3,0x0C,0x3E,0x83,0xCF,0x81,0x17,0xD9,0x22,0x5B,0x1F,
  0x80,0x41,0xF6,0xA3,0xAF,0xD5,0x08,0x93,0xD5,0x6B,0x23,0xCB,0x5E,0x6C,0x03,0x6F,
  0x28,0xAB,0x53,0x18,0x0F,0xA5,0xB1,0xDE,0x74,0x61,0x17,0xBC,0x8C,0xCE,0x96,0x2A,
  0x66,0xB5,0x57,0x4E,0x56,0xB6,0xAA,0x86,0xD7,0xF1,0x79,0x1A,0xF3,0xFC,0x02,0x4C,
  0x73,0xD9,0x8B,0xDE,0xCE,0xAD,0x88,0x84,0x51,0x3D,0x23,0xB9,0x27,0x71,0x17,0x2E,
  0xC7,0x4C,0xB2,0x36,0x97,0xB7,0xE0,0x00,0x28,0xBD,0x1C,0x95,0xB6,0x3A,0x83,0x4F,
  0x98,0x1E,0x4C,0x22,0x62,0xEA,0xA2,0xD8,0x85,0x8D,0x66,0x27,0xAA,0x28,0xC0,0x65,
  0x35,0xC9,0x92,0xBF,0x25,0x4D,0x2C,0xB1,0xD1,0x4A,0xD3,0x05,0xCE,0xBB,0x05,0x06,
  0xD8,0x2F,0x35,0x60,0x7B,0x16,0x32,0x67,0xFB,0xC0,0x54,0x11,0x4A,0xE3,0xB9,0x38,
  0x6A,0x33,0x5B,0xA1,0x60,0xB6,0xA3,0x30,0xAB,0x8D,0x8B,0x41,0x98,0x42,0x42,0x0B,
  0x66,0x2B,0x9E,0x4B,0x24,0x50,0x93,0xB8,0x93,0x8B,0x70,0x11,0xEB,0xD8,0x67,0x6F,
  0xEF,0xF5,0x5C,0x0A,0xAF,0xC2,0x28,0x2C,0x3A,0x7D,0x05,0x3B,0x70,0x32,0x67,0xF5,
  0x04,0x4E,0xC0,0x05,0x9C,0xC2,0x33,0x3C,0xBF,0x86,0x4B,0x6E,0xAD,0xED,0x2E,0xC0,
  0x79,0x9C,0xC0,0x73,0xB8,0xDA,0x78,0x43,0x3F,0x73,0x2E,0x0B,0x66,0x0A,0x61,0xE8,
  0x32,0xEB,0x72,0xB6,0x94,0x76,0xB2,0x29,0xBC,0x0C,0x87,0x4D,0xCA,0x7C,0x0C,0x60,
  0xEE,0x23,0xA1,0xEA,0xBD,0x81,0x17,0xF9,0xD4,0x8B,0xE6,0x19,0x35,0x30,0xCD,0x34,
  0x5D,0xA3,0x75,0x35,0x9A,0xAA,0x51,0x55,0xA3,0xB2,0x46,0x45,0x42,0xA7,0xF1,0x0E,
  0x2E,0xF1,0x01,0xE2,0x88,0x98,0xB3,0xC5,0x3B,0xB8,0x94,0xFE,0x31,0x84,0x30,0x0F,
  0xB0,0x89,0xC0,0x4C,0x83,0xC4,0x69,0x68,0xA2,0x56,0x51,0xA0,0xA5,0xFF,0x1A,0xAD,
  0xA2,0x89,0x56,0x91,0xD2,0xB7,0xC0,0x37,0xAF,0xC2,0xD3,0x3C,0x5B,0x78,0xE6,0xB8,
  0xAE,0x1B,0x29,0x83,0x9B,0x28,0xE0,0x1D,0x57,0xB3,0xE8,0x10,0x37,0x37,0x07,0xA5,
  0x93,0x51,0x17,0xA5,0x31,0x65,0x36,0xE0,0x4B,0xB4,0x51,0x6C,0x12,0x1D,0xE2,0x45,
  0xE1,0x6E,0xAF,0xE0,0x2A,0xD4,0x19,0x2F,0x82,0xC1,0x6E,0xEA,0xC0,0xD7,0xFC,0x38,
  0x4A,0xA2,0x18,0x2E,0xFB,0xAE,0x36,0x6A,0x44,0xF5,0x0E,0x09,0x9B,0xA0,0x16,0x78,
  0xCF,0x68,0xF0,0x1D,0x5A,0xB2,0x8C,0x1C,0x18,0xDC,0x2F,0xA6,0x70,0x3D,0xFB,0xD0,
  0xC0,0x6F,0x38,0xEF,0xEE,0x5D,0xFF,0xFB,0x3E,0x63,0x20,0xC1,0x4B,0x3D,0xBE,0xEB,
  0x7B,0xE5,0x6E,0xDA,0xC2,0x55,0x4F,0xE1,0x3B,0x62,0x14,0xEE,0xE3,0xEB,0xDC,0x0B,
  0xDD,0x95,0x19,0xB4,0x74,0xC2,0x9F,0x6F,0x60,0xC0,0x18,0xD5,0x3B,0x8B,0xB3,0x9C,
  0xD7,0x45,0xE6,0x13,0x18,0x23,0x87,0x75,0xCE,0xAB,0xCE,0xA2,0x43,0x81,0xEA,0x3D,
  0xEB,0x0B,0x68,0x67,0x54,0x40,0xDF,0xA7,0xFE,0x28,0xA3,0x65,0x5C,0x54,0x2B,0x96,
  0x2E,0xF9,0xDB,0xCD,0x07,0x74,0x0B,0x5B,0x68,0x3D,0x39,0x4B,0xDF,0x08,0x30,0x19,
  0x1C,0x77,0xFC,0xDE,0x71,0x31,0x56,0xF9,0x4A,0xB4,0xD3,0x9C,0xB5,0x3D,0xD7,0xA8,
  0x9D,0x07,0xFB,0xC7,0x96,0xF2,0xFA,0x5B,0x3A,0x84,0x5E,0x79,0x07,0x35,0x97,0x8B,
  0x62,0x06,0xA5,0x99,0x45,0xD6,0x20,0x6E,0xD3,0x64,0x65,0x1F,0x59,0x2D,0x51,0x62,
  0x17,0xCD,0xCD,0xC5,0xD1,0x6D,0xBA,0xC6,0x23,0x8D,0xBF,0xF9,0x19,0x3C,0x84,0xDF,
  0x99,0xFB,0x62,0x14,0xEF,0x92,0x8B,0x14,0xD9,0xFA,0x29,0xFA,0x89,0x3A,0xB1,0x5A,
  0x39,0x4F,0x33,0x6C,0xE9,0x14,0xFD,0xC2,0xBB,0x31,0xDE,0xCD,0x72,0x8D,0x60,0x30,
  0xAF,0xDB,0x6B,0x36,0x6F,0x8A,0x16,0x9A,0x67,0x6C,0x4F,0x3A,0xFC,0xB3,0xB2,0x4F,
  0xA4,0xC3,0x02,0x99,0x24,0x27,0xAA,0xC7,0xC9,0xA7,0xC5,0x55,0x6A,0x08,0x3B,0xB1,
  0x51,0x2E,0x38,0x02,0xE6,0x4B,0x72,0x11,0x37,0x70,0xBC,0x41,0xD0,0x89,0x4D,0x72,
  0x0A,0x73,0x37,0x3A,0xD0,0xC5,0xAD,0x7A,0x57,0x06,0x8C,0x6E,0x2A,0xD0,0x7C,0xA3,
  0x46,0x6C,0xF1,0x68,0x12,0xF5,0x62,0xD6,0xBB,0x86,0x35,0x2A,0xDD,0x16,0xB6,0x85,
  0xD3,0x74,0x94,0xB1,0xC2,0xD1,0xC0,0x55,0x5A,0xC7,0x3A,0x37,0xCB,0x02,0xE5,0x13,
  0x89,0xBB,0xA1,0xE4,0x9A,0x70,0xCB,0x91,0x7D,0xF4,0xBC,0xDC,0x76,0xE4,0x29,0xC9,
  0xB5,0x29,0xC3,0x90,0xD7,0xB7,0x33,0x50,0xFA,0x15,0xD9,0x10,0xD9,0xC8,0xEB,0x6D,
  0xE3,0xBC,0x7A,0xDA,0x8E,0x3C,0xAA,0xE0,0x70,0xF0,0xB8,0x82,0xE5,0xE0,0x71,0x05,
  0xDF,0x94,0xA3,0x50,0xA5,0xB7,0x82,0xBB,0x84,0x74,0x40,0xEE,0xA1,0x55,0xDC,0x73,
  0x8B,0xCD,0x62,0xE3,0xF4,0x1D,0x66,0x7D,0x07,0x25,0xF3,0x7B,0xDF,0x0B,0x1A,0x5C,
  0x3F,0xF3,0x74,0x3D,0xBF,0x8A,0x7B,0xF4,0xA0,0x54,0xBA,0x4A,0x1F,0x05,0xAE,0xF7,
  0x77,0x87,0xC7,0xF8,0xFD,0x87,0xF2,0x61,0x66,0x91,0xBE,0x90,0x0E,0x55,0xEE,0xDD,
  0xE7,0xC1,0x9E,0x30,0xCD,0x19,0x78,0xF8,0x0F,0xDC,0x1D,0x9E,0x09,0x46,0xB9,0x1E,
  0x67,0xE5,0x21,0xFE,0x17,0xED,0xA0,0xAC,0x3E,0xC1,0x5A,0xDE,0xE0,0xE8,0x0E,0xC8,
  0x38,0x5A,0x68,0x8E,0xE3,0x78,0x6E,0x06,0x15,0xD3,0xCB,0x41,0x96,0x63,0x97,0xDC,
  0xF7,0x57,0xA4,0x32,0x9F,0x31,0xEF,0xEA,0x3A,0x8E,0x00,0x6D,0x6C,0x7B,0x12,0x4F,
  0xE3,0x24,0x64,0xF8,0xDE,0xCD,0x60,0x7F,0x78,0x1A,0xAB,0xE4,0x45,0x3F,0x24,0x11,
  0xFC,0xC8,0x11,0x74,0xF2,0xBB,0xE3,0x58,0x8F,0xF7,0x02,0x4B,0xBF,0x06,0x82,0x3B,
  0xBC,0x0B,0x37,0xF0,0x1F,0xF3,0x7A,0x98,0xE2,0xB7,0xCF,0x9A,0x49,0xBC,0x27,0xDB,
  0x2B,0x69,0xDE,0x57,0x29,0x8F,0x8D,0x8C,0xAF,0x49,0x70,0xB8,0xFC,0x3D,0xB8,0x10,
  0x5A,0xFA,0x23,0xA8,0x52,0x77,0xB0,0x39,0x74,0x5E,0xC8,0x96,0x16,0xBE,0xB3,0x2C,
  0x68,0x0C,0xEB,0x54,0x95,0x66,0xFC,0x59,0x9A,0xC1,0x63,0xE4,0x6A,0xF2,0x7D,0xF8,
  0x40,0xC2,0xFF,0xDF,0x7F,0xF2,0x53,0x0B,0xFF,0x02,0x46,0xD6,0xE2,0x80,0x00,0x00,
  0x1A,0xFF,0xFF,0xFF,0x14,0x21,0x30,0x00,0x04,0x00,0x00,0x00,0x0F,0x00,0x00,0x00,
  0x1A,0xFF,0xFF,0xFF,0x20,0x20,0x30,0x00,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
//----------------------------------------------------------------------------
uint16_t FT8xx_Init_Goodix_GT911(uint16_t FWol)
  {
  //Make sure that the chip is caught up.
  Wait_for_FT8xx_Execution_Complete(FWol);

  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);

  //Pipe out data_length of data from data_address.
  const uint8_t
    *Flash_Data;
  uint32_t
    data_length;
  Flash_Data=Goodix_GT911_Init_Data;
  data_length=GOODIX_GT911_INIT_DATA_LENGTH;
  while(0 != data_length)
    {
    SPI.transfer(pgm_read_byte(Flash_Data));
    Flash_Data++;
    data_length--;
    }

  //Remember that we sent GOODIX_GT911_INIT_DATA_LENGTH bytes
  FWol=(FWol+GOODIX_GT911_INIT_DATA_LENGTH)&0xFFF;
  //De-select the FT8xx
  SET_FTDI_CS_NOT;

  //OK, the data is in the RAM_CMD circular buffer, ask the chip
  //to process it.
  FT8xx_REG_Write_16(REG_CMD_WRITE, FWol);
  //Now wait for it to catch up
  Wait_for_FT8xx_Execution_Complete(FWol);

  //"Straight AN_336"
  //"Hold the touch engine in reset (write REG_CPURESET=2)"
  FT8xx_REG_Write_8(REG_CPURESET, 0x02);
  //"Write REG_TOUCH_CONFIG=0x05D0"
  FT8xx_REG_Write_16(REG_TOUCH_CONFIG, 0x05D0);
  //"Set GPIO0 output LOW"
  // The CFA10100 FT813 uses GPIO3 to reset GT911
  // Reset-Value is 0x8000 adding 0x0008 sets GPIO3 to output
  // Default-value for REG_GPIOX is 0x8000 -> Low output on GPIO3
  FT8xx_REG_Write_16(REG_GPIOX_DIR,0x8008);
  // "Wait more than 100us"
  delay(1);
  // "Write REG_CPURESET=0"
  FT8xx_REG_Write_8(REG_CPURESET, 0x00);
  // "Wait more than 55ms"
  delay(56);
  // "Set GPIO0 to input (floating)"
  // The CFA10100 FT813 uses GPIO3 to reset GT911
  FT8xx_REG_Write_16(REG_GPIOX_DIR,0x8000);

  //Return the updated address
  return(FWol);
  }
#endif // (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE==CAP_DEV_GT911)
//============================================================================
//Apparently there is a "pen-up bug" in the FTDI.
#if (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE!=CAP_DEV_GT911)
//Magic binary data from FTDI supplied PC sample code
#define PEN_UP_BUG_FIX_INIT_DATA_LENGTH (1172)
const uint8_t Pen_Up_Bug_Fix_Init_Data[PEN_UP_BUG_FIX_INIT_DATA_LENGTH] PROGMEM = {
 26,255,255,255,32,32,48,0,4,0,0,0,2,0,0,0,34,255,255,255,0,176,48,0,120,218,237,84,255,107,92,69,16,159,125,155,107,141,201,121,247,106,130,9,225,244,238,37,246,146,52,63,53,98,172,53,48,243,8,36,166,182,63,136,216,246,7,205,219,75,122,119,185,139,196,128,34,33,136,123,65,240,7,31,44,105,41,69,72,12,210,166,6,17,12,104,64,165,73,133,134,98,80,74,43,148,22,20,133,40,20,34,233,15,82,250,131,10,113,246,229,106,197,191,161,111,217,55,179,59,59,59,243,153,47,251,135,15,58,204,11,109,114,89,149,84,169,242,172,2,109,10,73,53,167,92,158,89,21,6,89,230,236,126,86,89,206,242,169,178,61,187,115,62,85,190,199,133,129,44,132,249,90,12,243,124,67,49,169,76,81,22,195,60,211,113,40,133,249,253,104,114,113,148,197,131,105,169,158,3,19,132,65,28,187,240,25,112,3,147,99,235,163,80,98,63,10,123,181,208,73,213,29,24,246,226,121,112,199,146,74,98,56,22,195,135,245,32,14,194,17,180,182,100,46,169,220,114,244,103,171,114,252,41,222,143,97,150,53,64,167,202,110,240,86,186,45,16,90,14,191,158,110,196,163,216,130,241,232,159,42,251,88,4,169,90,80,42,163,226,248,168,206,97,132,136,169,69,113,20,27,245,203,40,131,12,54,87,37,143,253,43,105,97,137,9,172,103,181,26,172,79,163,102,184,142,57,83,0,61,139,144,159,79,135,193,62,61,0,157,250,5,20,186,67,91,255,133,238,140,104,167,30,192,197,72,2,85,137,221,89,134,101,140,177,191,238,137,75,105,137,23,97,18,226,31,175,98,31,174,165,141,90,193,21,88,194,117,60,207,243,115,248,202,254,171,171,37,248,20,87,112,17,59,180,59,246,35,199,177,83,175,35,140,93,224,187,172,45,169,172,108,29,47,192,59,186,70,159,1,208,155,136,40,71,190,195,101,222,117,131,221,90,142,110,48,221,85,165,177,42,173,169,82,89,165,78,149,138,136,110,224,13,108,200,0,132,1,50,103,114,55,176,145,254,51,132,208,183,177,137,64,111,128,192,13,104,162,86,145,165,198,255,141,86,209,68,79,146,84,87,33,163,95,132,39,120,182,242,76,115,78,123,40,142,7,200,227,21,103,50,103,171,109,123,27,242,115,193,0,197,176,70,63,141,125,212,35,14,8,95,28,20,246,116,155,30,132,152,118,3,40,29,162,62,60,158,9,131,8,197,120,49,99,115,35,39,228,200,168,48,81,197,2,175,221,224,205,140,173,148,4,87,13,148,94,19,235,216,205,62,212,179,14,199,221,234,21,223,207,112,254,163,90,169,197,247,50,110,249,16,249,156,113,137,83,98,146,163,23,215,115,16,146,173,102,142,158,74,160,205,181,28,217,67,93,156,121,40,156,202,76,130,169,184,193,12,9,125,82,132,149,121,209,65,219,198,12,47,80,7,15,182,163,97,52,171,206,211,20,186,229,195,148,42,15,137,45,116,244,29,188,138,105,177,76,139,149,18,113,141,160,192,1,218,222,14,131,101,90,224,17,195,239,51,113,156,194,111,245,21,49,137,171,100,61,238,21,166,50,67,95,83,63,154,202,71,116,141,45,205,208,37,94,133,188,186,201,177,134,82,86,117,84,109,46,137,52,237,214,102,248,22,253,82,41,137,91,148,37,29,97,147,195,86,126,89,156,165,45,214,188,43,54,43,160,63,36,139,169,142,17,73,175,31,255,230,232,164,175,244,161,197,38,71,118,121,70,109,226,43,252,14,100,249,142,135,180,156,48,185,45,172,176,247,71,244,29,104,240,25,3,37,216,78,179,159,101,171,9,63,225,95,166,6,126,71,24,29,156,165,199,253,126,108,119,88,51,146,216,19,191,137,102,191,107,186,68,109,78,247,244,95,130,99,94,134,49,183,128,158,84,207,58,245,129,9,220,145,78,16,218,85,102,226,94,102,142,121,247,51,243,146,119,63,51,3,14,71,157,94,245,86,169,151,134,156,99,212,206,189,148,208,9,109,239,43,242,125,57,135,249,19,111,120,245,182,79,121,218,62,110,231,222,203,57,220,245,52,237,217,126,246,253,183,89,95,59,59,145,238,165,15,28,91,45,86,239,140,215,231,199,56,34,59,239,154,239,159,244,38,57,63,167,156,41,126,95,14,83,82,189,203,183,244,51,186,33,39,12,234,244,105,156,79,111,161,100,186,232,37,25,187,195,253,252,137,35,245,2,243,54,207,243,8,176,143,109,175,225,57,92,131,71,248,220,151,222,113,255,28,26,114,131,139,17,130,111,24,65,63,235,157,198,90,188,238,213,251,63,120,130,59,151,187,130,95,193,207,252,26,214,221,175,215,240,186,211,85,217,197,107,67,123,113,15,217,42,252,201,155,245,133,94,131,122,255,87,207,80,143,215,238,91,47,126,247,106,248,204,159,153,187,116,219,49,116,45,115,147,182,48,242,4,190,240,127,118,224,193,247,224,3,89,247,15,148,99,211,103,26,255,255,255,20,33,48,0,4,0,0,0,15,0,0,0,26,255,255,255,32,32,48,0,4,0,0,0,0,0,0,0
};
//----------------------------------------------------------------------------
uint16_t FT8xx_Init_Pen_Up_Bug_Fix(uint16_t FWol)
  {
  //Make sure that the chip is caught up.
  Wait_for_FT8xx_Execution_Complete(FWol);

  //Combine Address_offset into  then select the FT8xx
  //and send the 24-bit address and operation flag.
  _FT8xx_Select_and_Address(RAM_CMD|FWol,MEM_WRITE);

  //Pipe out data_length of data from data_address.
  const uint8_t
    *Flash_Data;
  uint32_t
    data_length;
  Flash_Data=Pen_Up_Bug_Fix_Init_Data;
  data_length=PEN_UP_BUG_FIX_INIT_DATA_LENGTH;
  while(0 != data_length)
    {
    SPI.transfer(pgm_read_byte(Flash_Data));
    Flash_Data++;
    data_length--;
    }

  //Remember that we sent GOODIX_GT911_INIT_DATA_LENGTH bytes
  FWol=(FWol+PEN_UP_BUG_FIX_INIT_DATA_LENGTH)&0xFFF;
  //De-select the FT8xx
  SET_FTDI_CS_NOT;

  //OK, the data is in the RAM_CMD circular buffer, ask the chip
  //to process it.
  FT8xx_REG_Write_16(REG_CMD_WRITE, FWol);
  //Now wait for it to catch up
  Wait_for_FT8xx_Execution_Complete(FWol);

  //Return the updated address
  return(FWol);
  }
#endif // (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE!=CAP_DEV_GT911)
//============================================================================
#if TOUCH_TYPE==TOUCH_CAPACITIVE
//Returns a bit-mask of the points touched.
uint8_t Read_Touch(int16_t x_points[5], int16_t y_points[5])
  {
  uint32_t
    temp;

  //Capacitive
  temp = FT8xx_REG_Read_32(REG_CTOUCH_TOUCH0_XY);
  x_points[0]=(uint16_t)(temp>>16);
  y_points[0]=(uint16_t)temp;
  temp = FT8xx_REG_Read_32(REG_CTOUCH_TOUCH1_XY);
  x_points[1]=(uint16_t)(temp>>16);
  y_points[1]=(uint16_t)temp;
  temp = FT8xx_REG_Read_32(REG_CTOUCH_TOUCH2_XY);
  x_points[2]=(uint16_t)(temp>>16);
  y_points[2]=(uint16_t)temp;
  temp = FT8xx_REG_Read_32(REG_CTOUCH_TOUCH3_XY);
  x_points[3]=(uint16_t)(temp>>16);
  y_points[3]=(uint16_t)temp;

  //4th point is a special case
  x_points[4]=FT8xx_REG_Read_16(REG_CTOUCH_TOUCH4_X);
  y_points[4]=FT8xx_REG_Read_16(REG_CTOUCH_TOUCH4_Y);
  //Count up the points. 0x8000 means no touch.
  uint8_t
    points_touched_mask;
  points_touched_mask=0;

  uint8_t
    mask;
  mask=0x01;

  for(uint8_t i=0;i<5;i++)
    {
    if((0==(x_points[i]&0x8000))&&(0==(y_points[i]&0x8000)))
      {
      points_touched_mask|=mask;
      }
    mask<<=1;
    }
  return(points_touched_mask);
  }
#endif // TOUCH_TYPE==TOUCH_CAPACITIVE
//----------------------------------------------------------------------------
#if TOUCH_TYPE==TOUCH_RESISTIVE
//Returns a bit-mask of the points touched.
uint8_t Read_Touch(int16_t x_points[1], int16_t y_points[1])
  {
  uint32_t
    temp;
  //Resistive
  temp = FT8xx_REG_Read_32(REG_TOUCH_SCREEN_XY);
  x_points[0]=(uint16_t)(temp>>16);
  y_points[0]=(uint16_t)temp;
  //Count up the point.
  uint8_t
    points_touched_mask;
  points_touched_mask=0x00;
  if((0==(x_points[0]&0x8000))&&(0==(y_points[0]&0x8000)))
    {
    //We have a touch
    points_touched_mask=0x01; 
    }
  return(points_touched_mask);
  }
#endif // TOUCH_TYPE==TOUCH_RESISTIVE
//============================================================================
uint8_t FT8xx_Init(void)
  {
  // Wake up the FT8xx
  delay(20);          // Wait a few MS before waking the FT800
  CLR_FTDI_PD_NOT;    // 1) lower PD#
  delay(6);          // 2) hold for 20ms
  SET_FTDI_PD_NOT;    // 3) raise PD#
  delay(21);          // 4) wait for another 20ms before sending any commands

  // Start FTxx
  FT8xx_Command_Write_8(FT800_ACTIVE);
  //FT8xx_Command_Write_8(FT800_CLKEXT);      // Set FT800 for external clock
  //delay(50);          // Give some time to process
  //FT8xx_Command_Write_8(FT800_CLK48M);      // Set FT800 for 48MHz PLL
  //delay(50);          // Give some time to process
  // Now FT800 can accept commands at up to 30MHz clock on SPI bus

  //Poll REG_ID until the 0x7C vector comes back
  //Typically ~ 72mS Timeout at 250mS
  uint8_t
    ID_Timeout;
  uint8_t
    received_ID;
  ID_Timeout=250;
  while(0x7C != (received_ID=FT8xx_REG_Read_8(REG_ID)))
    {
    ID_Timeout--;
    if(0 == ID_Timeout)
      {
      // Send an error message on the UART
      Serial.print(F("After 250 tries have not received ID of 0x7C. Last recieved was 0x"));
      Serial.print(received_ID,HEX);
      Serial.print(F(" = "));
      Serial.println(received_ID);
      Serial.println(F("Is the device connected? Is the right FT81x vs FT80x selected?"));
      ID_Timeout=250;
      }
    else
      {
      delay(1);
      }
    }

  //Get the currrent write pointer offset from the FT81x
  uint16_t
    FWo;
  FWo = FT8xx_REG_Read_16(REG_CMD_WRITE);

#if (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE==CAP_DEV_GT911)
  FWo=FT8xx_Init_Goodix_GT911(FWo);
#endif // (TOUCH_TYPE==CAPACITIVE) && (TOUCH_CAP_DEVICE==CAP_DEV_GT911)

#if (TOUCH_TYPE==TOUCH_CAPACITIVE) && (TOUCH_CAP_DEVICE!=CAP_DEV_GT911)
  FWo=FT8xx_Init_Pen_Up_Bug_Fix(FWo);
#endif // (TOUCH_TYPE==CAPACITIVE) && (TOUCH_CAP_DEVICE!=CAP_DEV_GT911)

  // Now that we have some confidence we can talk to the FT8xx, proceed with
  // configuring it for our display.
  // Set PCLK to zero - don't clock the LCD until later
  FT8xx_REG_Write_8(REG_PCLK, 0);
  // Turn off backlight
  FT8xx_REG_Write_8(REG_PWM_DUTY, 0);

  // Initialize Display
  FT8xx_REG_Write_16(REG_HSIZE,   LCD_WIDTH);   // active display width
  FT8xx_REG_Write_16(REG_HCYCLE,  LCD_HCYCLE);  // total number of clocks per line, incl front/back porch
  FT8xx_REG_Write_16(REG_HOFFSET, LCD_HOFFSET); // start of active line
  FT8xx_REG_Write_16(REG_HSYNC0,  LCD_HSYNC0);  // start of horizontal sync pulse
  FT8xx_REG_Write_16(REG_HSYNC1,  LCD_HSYNC1);  // end of horizontal sync pulse
  FT8xx_REG_Write_16(REG_VSIZE,   LCD_HEIGHT);  // active display height
  FT8xx_REG_Write_16(REG_VCYCLE,  LCD_VCYCLE);  // total number of lines per screen, incl pre/post
  FT8xx_REG_Write_16(REG_VOFFSET, LCD_VOFFSET); // start of active screen
  FT8xx_REG_Write_16(REG_VSYNC0,  LCD_VSYNC0);  // start of vertical sync pulse
  FT8xx_REG_Write_16(REG_VSYNC1,  LCD_VSYNC1);  // end of vertical sync pulse
  FT8xx_REG_Write_8(REG_SWIZZLE,  LCD_SWIZZLE); // FT800 output to LCD - pin order
  FT8xx_REG_Write_8(REG_PCLK_POL, LCD_PCLKPOL); // LCD data is clocked in on this PCLK edge
  // Don't set PCLK yet - wait for just after the first display list

  //Set the LCD Drive to 10mA or 5mA 
  // REG_GPIOX
  // 1111 1100 0000 0000
  // 5432 1098 7654 3210
  // DGGP SSIr rrrr GGGG 
  // |||| |||| |||| ||||-- GPIO 0 PIN
  // |||| |||| |||| |||--- GPIO 1 PIN
  // |||| |||| |||| ||---- GPIO 2 PIN
  // |||| |||| |||| |----- GPIO 3 PIN
  // |||| |||| ||||------- (reserved)
  // |||| |||------------- INT_N: 0=Open Drain, 1 = default
  // |||| ||-------------- SPI drive: 00=5mA, 01=10mA, 10=15mA, 11=20mA
  // ||||----------------- PCLK+RGB Drive: 0=4mA, 1=10mA
  // |||------------------ GPIO drive:  00=5mA, 01=10mA, 10=15mA, 11=20mA
  // |-------------------- DISP PIN
#if (0 != LCD_DRIVE_10MA)
  //Set 10mA drive for:
  //  PCLK, DISP , VSYNC, HSYNC, DE, RGB lines & BACKLIGHT
  FT8xx_REG_Write_16(REG_GPIOX,FT8xx_REG_Read_16(REG_GPIOX) | 0x1000);
#else  
  //Set 5mA drive for:
  //  PCLK, DISP , VSYNC, HSYNC, DE, RGB lines & BACKLIGHT
  FT8xx_REG_Write_16(REG_GPIOX,FT8xx_REG_Read_16(REG_GPIOX) & ~0x1000);
#endif

#if (0 != LCD_PCLK_CSPREAD)
  FT8xx_REG_Write_8(REG_CSPREAD,1);
#else
  FT8xx_REG_Write_8(REG_CSPREAD,0);
#endif
#if (0 != LCD_DITHER)
  FT8xx_REG_Write_8(REG_DITHER,1);
#else
  FT8xx_REG_Write_8(REG_DITHER,0);
#endif
  // RGB 6-6-6 (default for FT810/FT811)
  //FT8xx_REG_Write_16(REG_OUTBITS,0x1B6);
  // RGB 8-8-8 (default for FT812/FT813)
  //FT8xx_REG_Write_16(REG_OUTBITS,0x000);

#if (TOUCH_TYPE==TOUCH_NONE)
  // Disable touch
  FT8xx_REG_Write_8(REG_TOUCH_MODE, 0);
  // Eliminate any false touches
  FT8xx_REG_Write_16(REG_TOUCH_RZTHRESH, 0);
#endif // (TOUCH_TYPE==TOUCH_NONE)

#if (TOUCH_TYPE==TOUCH_RESISTIVE)
  //Resistive.
  //Set the threshold somewhere reasonable.
  FT8xx_REG_Write_16(REG_TOUCH_RZTHRESH, 1200);
  //Oversample - higher current better resolution
  FT8xx_REG_Write_8(REG_TOUCH_OVERSAMPLE, 6);
  //Touch on, once per frame
  FT8xx_REG_Write_8(REG_TOUCH_MODE, TOUCH_MODE_FRAME);
#endif // (TOUCH_TYPE==TOUCH_RESISTIVE)

#if (TOUCH_TYPE==TOUCH_CAPACITIVE)
  //Capacitive.
  //Touch on, at rate of touch IC
  FT8xx_REG_Write_8(REG_CTOUCH_MODE, CTOUCH_MODE_CONTINUOUS);
  //Set compatibility (single touch) mode until after touch cal.
  FT8xx_REG_Write_8(REG_CTOUCH_EXTEND, CTOUCH_EXTEND_COMPATIBILITY);
#endif // (TOUCH_TYPE==TOUCH_CAPACITIVE)

  // Turn recorded audio volume down & stop
  FT8xx_REG_Write_8(REG_VOL_PB, 0);
  FT8xx_REG_Write_32(REG_PLAYBACK_PLAY,0);
  // Turn synthesizer volume down
  FT8xx_REG_Write_8(REG_VOL_SOUND, 0);
  // Set synthesizer to mute
  FT8xx_REG_Write_16(REG_SOUND, 0x0060);
  FT8xx_REG_Write_8(REG_PLAY,1);

  // Create and write an Initial Display List & Enable Display
  // Set the default clear color to black
  FWo=FT8xx_Cmd_Dat_0(FWo,
                      CLEAR_COLOR_RGB(0,0,0));
  // Clear the screen - this and the previous prevent artifacts between lists
  FWo=FT8xx_Cmd_Dat_0(FWo,
                      CLEAR(1 /*CLR_COL*/,1 /*CLR_STN*/,1 /*CLR_TAG*/));

   // Instruct the graphics processor to show the list
  FWo=FT8xx_Cmd_Dat_0(FWo,
                      DISPLAY());
  // Make this list active
  FWo=FT8xx_Cmd_Dat_0(FWo,
                      CMD_SWAP);

  // Instruct the graphics processor to show the list
  // Note: In this first instance it is a register write, not a command in the list.
  FT8xx_REG_Write_32(REG_DLSWAP, DLSWAP_FRAME);

  // Nothing is being displayed yet... the pixel clock is still 0x00

  // Enable the DISP line of the LCD.
  // REG_GPIOX
  // 1111 1100 0000 0000
  // 5432 1098 7654 3210
  // DGGP SSIr rrrr GGGG 
  // |||| |||| |||| ||||-- GPIO 0 PIN
  // |||| |||| |||| |||--- GPIO 1 PIN
  // |||| |||| |||| ||---- GPIO 2 PIN
  // |||| |||| |||| |----- GPIO 3 PIN
  // |||| |||| ||||------- (reserved)
  // |||| |||------------- INT_N: 0=Open Drain, 1 = default
  // |||| ||-------------- SPI drive: 00=5mA, 01=10mA, 10=15mA, 11=20mA
  // ||||----------------- PCLK+RGB Drive: 0=4mA, 1=10mA
  // |||------------------ GPIO drive:  00=5mA, 01=10mA, 10=15mA, 11=20mA
  // |-------------------- DISP PIN
  FT8xx_REG_Write_16(REG_GPIOX,FT8xx_REG_Read_16(REG_GPIOX) | 0x8000);

  // Now start clocking data to the LCD panel, enabling the display
  FT8xx_REG_Write_8(REG_PCLK, LCD_PCLK);

  //Backlight frequency default is 250Hz. That is fine for CFA10100
  FT8xx_REG_Write_16(REG_PWM_HZ,250);
  //CFA10100 circuit has soft start. No need to ramp
  FT8xx_REG_Write_8(REG_PWM_DUTY,64);
  
  //Signal success.
  return(0);
  }
//===========================================================================
