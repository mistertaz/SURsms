///////////////////////////////////////////////////////////////////////////
////                       pcd_bootloader.h                            ////
////                                                                   ////
////  This include file must be included by any application loaded     ////
////  by the example bootloader (ex_pcd_bootloader.c).                 ////
////                                                                   ////
////  The directives in this file relocates the reset vector as well   ////
////  as reserving space for the bootloader.                           ////
///////////////////////////////////////////////////////////////////////////
////        (C) Copyright 1996,2014 Custom Computer Services           ////
//// This source code may only be used by licensed users of the CCS    ////
//// C compiler.  This source code may only be distributed to other    ////
//// licensed users of the CCS C compiler.  No other use,              ////
//// reproduction or distribution is permitted without written         ////
//// permission.  Derivative programs created using this software      ////
//// in object code form are not restricted in any way.                ////
///////////////////////////////////////////////////////////////////////////
#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#ifndef LOADER_PAGES
 #if getenv("FLASH_ERASE_SIZE") == 128
  #define LOADER_PAGES 19
 #else
  #define LOADER_PAGES 2
 #endif
#endif

#ifndef BOOTLOADER_AT_START
 #define BOOTLOADER_AT_START
#endif

#define PROGRAM_MEMORY_FIND_PAGE_START(address) ((__ADDRESS__)address & ~(((__ADDRESS__)getenv("FLASH_ERASE_SIZE")/2)-(__ADDRESS__)1))

// getenv("PROGRAM_MEMORY") won't return the TRUE size as it removes some things like config bits.
// this define gives TRUE program memory size
#define PROGRAM_MEMORY_SIZE   PROGRAM_MEMORY_FIND_PAGE_START(getenv("PROGRAM_MEMORY")) + (getenv("FLASH_ERASE_SIZE")/2)

#ifdef BOOTLOADER_AT_START
 #define LOADER_SIZE ((LOADER_PAGES * (getenv("FLASH_ERASE_SIZE")/2)) - 1)

 #define LOADER_END              LOADER_SIZE
 #define LOADER_ADDR             0
 #define APPLICATION_START       (LOADER_END + 1)
 #define APPLICATION_END         (PROGRAM_MEMORY_SIZE-1)
 #define APPLICATION_ISR_START   (LOADER_END + 5)
#else
 #if ((getenv("PROGRAM_MEMORY") % (getenv("FLASH_ERASE_SIZE")/2)) !=0 )
  #define LOADER_SIZE (((getenv("PROGRAM_MEMORY") % (getenv("FLASH_ERASE_SIZE")/2)) - 1) + ((LOADER_PAGES - 1) * (getenv("FLASH_ERASE_SIZE")/2)))
 #else
  #define LOADER_SIZE ((LOADER_PAGES * (getenv("FLASH_ERASE_SIZE")/2)) - 1)
 #endif
 #define LOADER_END  (getenv("PROGRAM_MEMORY") - 1)
 #define LOADER_ADDR (PROGRAM_MEMORY_SIZE - LOADER_SIZE)
 #define APPLICATION_START 0
 #define APPLICATION_END   (LOADER_ADDR-1)
#endif

#ifndef _bootloader
 #ifdef BOOTLOADER_AT_START
  #build (reset=APPLICATION_START,interrupt=APPLICATION_ISR_START)
 #endif
 #org LOADER_ADDR, LOADER_END {}
#else
 #ifdef BOOTLOADER_AT_START
   #org APPLICATION_START, (getenv("PROGRAM_MEMORY")-1) {}
 #else
   #org 0, APPLICATION_START-1
 #endif
#endif

#endif
