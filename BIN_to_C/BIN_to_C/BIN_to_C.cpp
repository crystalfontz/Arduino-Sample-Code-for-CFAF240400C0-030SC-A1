//=============================================================================
//
//  CRYSTALFONTZ BIN to C Source initialized array for flash.
//
//  Visual Studio 2017
//
//  ref: https://www.crystalfontz.com/
//
//  2019-01-18 Brent A. Crosby
//
//=============================================================================
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
//=============================================================================
//
// To make this static linked (no DLLs required)
//
//ref: https://social.msdn.microsoft.com/Forums/vstudio/en-US/845617aa-ab76-4fe1-a658-d020828598b8/static-linking-in-vs2010-runtime?forum=vcgeneral
//
//To set static linking:
//  Configuration Properties, C/C++, Code Generation, Runtime Library -> MultiThreaded (/MT)
//In addition, if using MFC:
//  Configuration Properties, General, Use of MFC -> Use MFC in a Static Library
//In addition, if using ATL:
//  Configuration Properties, General, Use of ATL -> Static Link to ATL
//
//(this project is already edited for wide strings, so do not do this)
//So the normal strings will work:
//  Configuration Properties, General, Character Set -> Use Multi-Byte Character Set
//
//Less paranoid:
//  Configuration Properties, C/C++, Preprocessor, Preprocessor Definitions, Add _CRT_SECURE_NO_WARNINGS
//
//=============================================================================
#include "pch.h"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <tchar.h>
//===========================================================================
uint8_t *LoadBinaryFile(wchar_t *filename, uint32_t *dataLength)
  {
  FILE
    *filePtr;
  //We will allocate memory for the binary image data at this pointer.
  uint8_t
    *binaryData;
  size_t
    bytes_read_from_binaryfile;
  errno_t
    file_open_err;

  //Default to no data read.
  *dataLength = 0;

  //open filename in read binary mode
  file_open_err = _wfopen_s(&filePtr, filename, L"rb");
  if (file_open_err)
    {
    wprintf(L"Error# %d: %hs reading the bn=inady file in: \"%Ts\"\n", file_open_err, strerror(file_open_err), filename);
    return(NULL);
    }

  //Get the size of the file
  uint32_t
    file_size;
  fseek(filePtr, 0L, SEEK_END);
  file_size = ftell(filePtr);
  fseek(filePtr, 0L, SEEK_SET);

  //allocate enough memory for the bitmap image data
  binaryData = (uint8_t*)malloc(file_size);

  //verify memory allocation
  if (NULL == binaryData)
    {
    fclose(filePtr);
    return(NULL);
    }

  //read in the bitmap image data
  bytes_read_from_binaryfile =
    fread(binaryData, 1, file_size, filePtr);

  //make sure bitmap image data was read
  if (bytes_read_from_binaryfile != file_size)
    {
    free(binaryData);
    fclose(filePtr);
    return(NULL);
    }

  //close file
  fclose(filePtr);
  //return binary data size and pointer
  *dataLength = file_size;
  return(binaryData);
  }
//===========================================================================
void usage(wchar_t *our_name)
  {
  wprintf(L"Usage:\n");
  wprintf(L"  %s [infile] [outfile]\n", our_name);
  wprintf(L"  [infile] is binary file\n");
  wprintf(L"  [outfile] is is C source array\n");
  }
//===========================================================================
//int main(int argc, char *argv[])
int _tmain(int argc, _TCHAR*  argv[])
  {
  //Must have 3 parameters
  if (3 != argc)
    {
    usage(argv[0]);
    return(1);
    }

  wprintf(L"Reading input binary file: %s\n", argv[1]);
  //Crack open the file file, allocate memory and the data
  uint8_t
    *binaryData;
  uint32_t
    binaryData_length;
  binaryData = LoadBinaryFile(argv[1],&binaryData_length);

  //Check that the operation went OK.
  if ((NULL == binaryData) || (0 == binaryData_length))
    {
    wprintf(L"Error reading/processing the BMP.\n");
    usage(argv[0]);
    return(1);
    }

  wprintf(L"Data Length = %d\n", binaryData_length);

  //Now we need to print the data out as a C array
  errno_t
    file_open_err;
  FILE
    *Out_C_Array_File;


  file_open_err = _wfopen_s(&Out_C_Array_File, argv[2], L"wt");
  if (file_open_err)
    {
    wprintf(L"Error# %d: %s opening the C arrray output file: \"%s\"\n", file_open_err, strerror(file_open_err), argv[2]);
    usage(argv[0]);
    //free the memory allocated by LoadBitmapFile
    free(binaryData);
    //Indcicate failure, exit
    return(NULL);
    }
  wprintf(L"Opened \"%s\" as the C array output file.\n", argv[2]);



  //Now we just loop down the file, line by line (bottom first),
  //create the 16-bit words and spool them out.
  int
    col;
  int
    row;

  //Our pointer into the bitmap data
  uint8_t
    *data_pointer;

  //Write out the header information to the file.  This is foramatted for Arduino (sorry)
  fwprintf(Out_C_Array_File, L"//Source Binary file: \"%s\"\n", argv[1]);
  fwprintf(Out_C_Array_File,
          L"const uint8_t SOME_DATA[ %d ] PROGMEM =\n  {\n  ",
          binaryData_length);
  //Write 16 numbers per line.
  uint32_t
    column;
  column=0;

  //Loop through the data
  for (uint32_t i = 0; i < binaryData_length; i++)
    {
    //indent on first column
//    if (0 == column)
//      fwprintf(Out_C_Array_File, L"  ");
    //Write this byte
    fwprintf(Out_C_Array_File, L"0x%02X", *binaryData);
    //Point to the next byte
    binaryData++;
    //If it is the last byte in the file, close the initialization data
    if (i == (binaryData_length -1))
      {
      fwprintf(Out_C_Array_File, L"\n  };\n");
      }
    else
      {
      //If it is the last column, move to the next line & indent
      if (15 == column)
        {
        fwprintf(Out_C_Array_File, L",\n  ");
        column = 0;
        }
      //Otherwise just put the comma divider
      else
        {
        fwprintf(Out_C_Array_File, L",");
        column++;
        }
      }
    }

  //Done with the output file.
  fclose(Out_C_Array_File);

  //free the memory allocated by LoadBitmapFile
  free(binaryData);

  wprintf(L"Complete.\n");

  //Indicate success, exit
  return(0);
  }
//===========================================================================
