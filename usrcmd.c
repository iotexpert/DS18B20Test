/**
 * @file usrcmd.c
 * @author CuBeatSystems
 * @author Shinichiro Nakamura
 * @copyright
 * ===============================================================
 * Natural Tiny Shell (NT-Shell) Version 0.3.1
 * ===============================================================
 * Copyright (c) 2010-2016 Shinichiro Nakamura
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */


#include "ntopt.h"
#include "ntlibc.h"
#include "ntshell.h"
#include <stdio.h>
#include "owb.h"
#include "cybsp.h"
#include "task.h"

extern ntshell_t ntshell;

typedef int (*USRCMDFUNC)(int argc, char **argv);

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj);
static int usrcmd_help(int argc, char **argv);
static int usrcmd_info(int argc, char **argv);
static int usrcmd_clear(int argc, char **argv);
static int usrcmd_printargs(int argc, char **argv);
static int usrcmd_init(int argc, char **argv);
static int usrcmd_reset(int argc, char **argv);
static int usrcmd_read(int argc, char **argv);
static int usrcmd_write(int argc, char **argv);
static int usrcmd_owbw(int argc, char **argv);
static int usrcmd_owbwb(int argc, char **argv);

static int usrcmd_readb(int argc, char **argv);
static int usrcmd_readrom(int argc, char **argv);
static int usrcmd_readtemp(int argc, char **argv);



typedef struct {
    char *cmd;
    char *desc;
    USRCMDFUNC func;
} cmd_table_t;

static const cmd_table_t cmdlist[] = {
    { "help", "This is a description text string for help command.", usrcmd_help },
    { "info", "This is a description text string for info command.", usrcmd_info },
    { "clear", "Clear the screen", usrcmd_clear },
    { "printargs","print the list of arguments", usrcmd_printargs},
    { "init","Initialize the 1-wire bus", usrcmd_init},
    { "reset","Reset the 1-wire bus", usrcmd_reset},
    { "read","Reset the 1-wire bus", usrcmd_read},
    { "write","write [0|1]", usrcmd_write},
    { "owbw","write bit: owbw [0|1]", usrcmd_owbw},
    { "owbwb","write byte: owbw [0|1]", usrcmd_owbwb},
    { "readb","Read", usrcmd_readb},
    { "readrom","readrom", usrcmd_readrom},
  { "temp","temp", usrcmd_readtemp},

};

int usrcmd_execute(const char *text)
{
    return ntopt_parse(text, usrcmd_ntopt_callback, 0);
}

static int usrcmd_ntopt_callback(int argc, char **argv, void *extobj)
{
    if (argc == 0) {
        return 0;
    }
    const cmd_table_t *p = &cmdlist[0];
    for (unsigned int i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
        if (ntlibc_strcmp((const char *)argv[0], p->cmd) == 0) {
            return p->func(argc, argv);
        }
        p++;
    }
    printf("%s","Unknown command found.\n");
    return 0;
}

static int usrcmd_help(int argc, char **argv)
{
    const cmd_table_t *p = &cmdlist[0];
    for (unsigned int i = 0; i < sizeof(cmdlist) / sizeof(cmdlist[0]); i++) {
        printf("%s",p->cmd);
        printf("%s","\t:");
        printf("%s",p->desc);
        printf("%s","\n");
        p++;
    }
    return 0;
}


static int usrcmd_info(int argc, char **argv)
{
    if (argc != 2) {
        printf("%s","info sys\n");
        printf("%s","info ver\n");
        return 0;
    }
    if (ntlibc_strcmp(argv[1], "sys") == 0) {
        printf("%s","PSoC 6 MBED Monitor\n");
        return 0;
    }
    if (ntlibc_strcmp(argv[1], "ver") == 0) {
        printf("%s","Version 0.0.0\n");
        return 0;
    }
    printf("%s","Unknown sub command found\n");
    return -1;
}


static int usrcmd_clear(int argc, char **argv)
{
    vtsend_erase_display_home(&ntshell.vtsend);
    return 0;
}

static int usrcmd_printargs(int argc, char **argv)
{
    printf("ARGC = %d\n",argc);

    for(int i =0;i<argc;i++)
    {
        printf("argv[%d] = %s\n",i,argv[i]);
    }
    return 0;

}

OneWireBus bus;
static int usrcmd_init(int argc, char **argv)
{
    bus.pin = CYBSP_D4;
    owb_init(&bus);
    printf("Initialized D4\n");

    return 0;
}

static int usrcmd_reset(int argc, char **argv)
{
    bool result;
    owb_ret_t rval = owb_reset(&bus,&result);
    if(rval == OWB_STATUS_OK)
    {
        printf("Reset Succedded\n");

    }
    else

        printf("Reset Failed\n");

    return 0;
}

static int usrcmd_read(int argc, char **argv)
{
    printf("Val = %d\n",cyhal_gpio_read(bus.pin));
    return 0;
}

static int usrcmd_write(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Invalid argument write  %d\n",argc);
        return 0;
    }
    
    printf("Argv = %s\n",argv[1]);
    switch(argv[1][0])
    {
        case '0':
            printf("Write 0\n");
            cyhal_gpio_write(bus.pin,0);
        break;
        case '1':
            printf("Write 1\n");
                cyhal_gpio_write(bus.pin,1);
        break;
    }

    return 0;
}

static int usrcmd_owbw(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Invalid argument write  %d\n",argc);
        return 0;
    }

    owb_ret_t ret=OWB_STATUS_OK;
    
    switch(argv[1][0])
    {
        case '0':
            printf("owbw 0\n");
            ret = owb_write_bit(&bus,0);

        break;
        case '1':
            printf("Write 1\n");
            ret = owb_write_bit(&bus,1);
        break;
    }

    if(ret == OWB_STATUS_OK)
        printf("Write succeded\n");
    else
        printf("Write failed\n");


    return 0;
}

static int usrcmd_owbwb(int argc, char **argv)
{
    if(argc != 2)
    {
        printf("Invalid argument write  %d\n",argc);
        return 0;
    }

    owb_ret_t ret=OWB_STATUS_OK;

    int val;
    sscanf(argv[1],"%02x",&val);

    printf("Write Byte Val = %02X\n",val);

    ret = owb_write_byte(&bus,(uint8_t)val);

    if(ret == OWB_STATUS_OK)
        printf("Write byte succeded\n");
    else
        printf("Write byte failed\n");


    return 0;
}

static int usrcmd_readb(int argc, char **argv)
{

    owb_ret_t ret=OWB_STATUS_OK;

    uint8_t val;
 
    ret = owb_read_byte(&bus,&val);

    if(ret == OWB_STATUS_OK)
        printf("Read byte succeded %d\n",val);
    else
        printf("Read byte failed\n");


    return 0;
}

uint8_t rom[8];

static int usrcmd_readrom(int argc, char **argv)
{
    owb_ret_t rval;

    printf("Sending reset\n");
    bool result;
    rval = owb_reset(&bus,&result);
    if(rval != OWB_STATUS_OK)
    {
        printf("Reset failed\n");
        return 0;
    }

    CyDelay(1);

    printf("Sending 0x33\n");

    rval = owb_write_byte(&bus,0x33);
    if(rval != OWB_STATUS_OK)
    {
        printf("Write 0x33 failed\n");
        return 0;
    }


    // read 64-bit rom
    printf("Reading 8 bytes\n");

    rval = owb_read_bytes(&bus,rom,8);

   if(rval != OWB_STATUS_OK)
    {
        printf("read 8 bytes failed\n");
        return 0;
    }
    // 
    printf("Rom = ");
    for(int i=0;i<8;i++)
    {
        printf("%02X ",rom[i]);
    }
    printf("\n");
    return 0;
}

static int usrcmd_readtemp(int argc, char **argv)
{
    uint8_t scratchpad[9];
    bool result;

    // send reset
    // send match rom 0x55
    // send rom address
    // send trigger 0x44
    // delay 1 second
    // send reset
    // send match rom 0x55
    // send read scratchpad BEh
    // read 9 bytes of scratch pad

    owb_reset(&bus,&result);
    CyDelay(1);
    owb_write_byte(&bus,0x55);
    owb_write_bytes(&bus,rom,8);
    owb_write_byte(&bus,0x44);
    vTaskDelay(1000);
    owb_reset(&bus,&result);
    CyDelay(1);

    owb_write_byte(&bus,0x55);
    owb_write_bytes(&bus,rom,8);
    owb_write_byte(&bus,0xbe);
    owb_read_bytes(&bus,scratchpad,9);
    printf("Scratchpad =");
    for(int i=0;i<9;i++)
    {
        printf("%02X ",scratchpad[i]);
    }
    printf("\n");
 
    int16_t tempbits = scratchpad[0] | (scratchpad[1] << 8);
    uint32_t resolution=2^12;
    float temperature=0.0;
    switch(scratchpad[5] >>5 & 0x03)
    {
        case 0:
            resolution = 2^9;
            break;
        case 1:
            resolution = 2^10;
            break;
        case 2:
            resolution = 2^11;
            break;
        case 3:
            resolution = 2^12;
        break;
    }

    temperature = (float)tempbits / (float)resolution;

    printf("Temperature = %f\n",temperature);
    return 0;  

}