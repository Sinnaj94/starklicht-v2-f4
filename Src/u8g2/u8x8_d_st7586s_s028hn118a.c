/*
  u8x8_d_st7586s_s028hn118a.c
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  Copyright (c) 2018, olikraus@gmail.com
  All rights reserved.
  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:
  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.
  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  
*/

#include <u8g2/u8g2.h>


static const uint8_t u8x8_d_st7586s_sleep_on[] = {
  U8X8_START_TRANSFER(),  /* enable chip, delay is part of the transfer start */
  U8X8_C(0x010), /* set power save mode */
  U8X8_END_TRANSFER(),  /* disable chip */
  U8X8_END()                  /* end of sequence */
};

static const uint8_t u8x8_d_st7586s_sleep_off[] = {
  U8X8_START_TRANSFER(),  /* enable chip, delay is part of the transfer start */
  U8X8_C(0x011), //Sleep out
  U8X8_DLY(50), /* delay 50 ms */
  U8X8_END_TRANSFER(),  /* disable chip */
  U8X8_END()                  /* end of sequence */
};

static const uint8_t u8x8_d_st7586s_s028hn118a_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x036),				/* Scan Direction Setting */
  U8X8_A(0x000),				/* COM0 -> COM159 SEG0 -> SEG384 */
  U8X8_C(0x037),				/* Start line 0 */
  U8X8_A(0x000),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7586s_s028hn118a_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x036),				/* Scan Direction Setting */
  U8X8_A(0x0C8),				/* COM159 -> COM0 SEG384 -> SEG0 */
  U8X8_C(0x037),				/* Start line 24 */
  U8X8_A(0x018),
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static uint8_t u8x8_d_st7586s_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  uint8_t c;
  uint8_t *ptr;
  uint8_t i, byte;
  uint32_t input;
  uint8_t output[8];
  switch (msg) {
  case U8X8_MSG_DISPLAY_DRAW_TILE:
    u8x8_cad_StartTransfer(u8x8); // OK Start transfer
    u8x8_cad_SendCmd(u8x8, 0x02B);  /* Row Address Setting */
    u8x8_cad_SendArg(u8x8, 0x000);
    u8x8_cad_SendArg(u8x8, 0x008 * ((u8x8_tile_t *)arg_ptr)->y_pos);
    u8x8_cad_SendArg(u8x8, 0x000);
    u8x8_cad_SendArg(u8x8, u8x8->display_info->pixel_height - 1);  /* should this be u8x8->display_info->pixel_height - 1 */
    u8x8_cad_SendCmd(u8x8, 0x02C);  /* cmd write display data to ram */
    c = ((u8x8_tile_t *) arg_ptr)->cnt; //
    c *= 8;
    ptr = ((u8x8_tile_t *) arg_ptr)->tile_ptr;  //

// The ST7586S has an unusual 3 pixels per byte format so here we read in 3 bytes (24 pixels) and
// pack that into 8 bytes of 3 pixels each 	
	while (c > 0) {
      input = (((uint32_t)ptr[0] << 16) | ((uint32_t)ptr[1] << 8) | (uint32_t)ptr[2]);
      for (i=0; i<8; i++)
      {
        byte = 0;
        if (input & 0x800000)          // if bit 23
            byte = byte | 0xC0;  //set pixel 1
        if (input & 0x400000)          // if bit 22
            byte = byte | 0x18;  //set pixel 2
		if (input & 0x200000)          // if bit 22
			byte = byte | 0x3;  //set pixel 3
		output[i] = byte;
        input <<= 3;
      }
      u8x8_cad_SendData(u8x8, 8, output);
      ptr += 3;
      c -= 3;
    }
    u8x8_cad_EndTransfer(u8x8);
    break;
  case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
    if (arg_int == 0)
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7586s_sleep_off);
    else
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7586s_sleep_on);
    break;
#ifdef U8X8_WITH_SET_CONTRAST
  case U8X8_MSG_DISPLAY_SET_CONTRAST:
    u8x8_cad_StartTransfer(u8x8);
    u8x8_cad_SendCmd(u8x8, 0x0C0);
    u8x8_cad_SendArg(u8x8, arg_int);
    u8x8_cad_SendArg(u8x8, 0);
    u8x8_cad_EndTransfer(u8x8);
    break;
#endif
  default:
    return 0;
  }
  return 1;
}

static const uint8_t u8x8_d_st7586s_s028hn118a_init_seq[] = {
  U8X8_END_TRANSFER(),/* disable chip */
//  U8G_ESC_RST(15), /* hardware reset */
  U8X8_DLY(60),   /* Delay 60 ms */
  U8X8_START_TRANSFER(),/* enable chip */

  U8X8_C(0x001), // Soft reset
  U8X8_DLY(60), // Delay 120 ms

  U8X8_C(0x011), // Sleep Out
  U8X8_C(0x028), // Display OFF
  U8X8_DLY(25), // Delay 50 ms

  U8X8_CAA(0x0C0,0x0E5,0x00),// Vop = F0h in trace a bit too dark

  U8X8_CA(0x0C3,0x004), // BIAS = 1/10 0x04 in trace

  U8X8_CA(0x0C4,0x005), // Booster = x6 0x05 in trace

  U8X8_CA(0x0D0,0x01D), // Enable Analog Circuit

  U8X8_CA(0x0B3,0x000), // Set FOSC divider

  U8X8_CA(0x0B5,0x08B), // N-Line = 0

  U8X8_C(0x039), // 0x39 Monochrome mode. 0x38 - gray Mode

  U8X8_C(0x03A), // Enable DDRAM Interface
  U8X8_A(0x002), // monochrome and 4-level

  U8X8_C(0x036), // Scan Direction Setting
  U8X8_A(0x000), // COM0 -> COM159 SEG0 -> SEG384

  U8X8_C(0x0B0), // Duty Setting (num rows - 1)
  U8X8_A(0x087), // should be 0x87 but caused flicker 0x9F

  U8X8_C(0x020), // Display inversion off

  U8X8_C(0x02A), // Column Address Setting
  U8X8_A(0x000), // COL0 -> COL127
  U8X8_A(0x000), // 
  U8X8_A(0x000), //
  U8X8_A(0x07f), // 128*3=384 pixels

  U8X8_C(0x02B), // Row Address Setting
  U8X8_A(0x000), // ROW0 -> ROW135
  U8X8_A(0x000), //
  U8X8_A(0x000), //
  U8X8_A(0x087), // 136 pixels

  U8X8_C(0x0F1), // Frame rate monochrome
  U8X8_A(0x00C), // The factory firmware set this to 49.0 Hz 0x07
  U8X8_A(0x00C), // This caused a shimmer under 50Hz LED lights
  U8X8_A(0x00C), // 69.0 Hz (0x0C) fixes this and should avoid the
  U8X8_A(0x00C), // issue in the US too
  
  U8X8_C(0x029), // Display ON
  U8X8_END()  /* end of sequence */
};

static const u8x8_display_info_t u8x8_st7586s_s028hn118a_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,

  /* post_chip_enable_wait_ns = */ 5,
  /* pre_chip_disable_wait_ns = */ 5,
  /* reset_pulse_width_ms = */ 1,
  /* post_reset_wait_ms = */ 6,
  /* sda_setup_time_ns = */ 20,
  /* sck_pulse_width_ns = */  100,  /* datasheet ST7586S */
  /* sck_clock_hz = */ 8000000UL, /* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* ST7586+Atmega128RFA1 works with 8MHz */
  /* spi_mode = */ 3,   /* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 20, /* datasheet suggests min 20 */
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 48,
  /* tile_height = */ 17,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 384,
  /* pixel_height = */ 136
};

/*******************************************************************************
 * st7586s_s028hn118a driver. This is the display in the SMART Response XE. This requires 16 bit mode.
 ******************************************************************************/
uint8_t u8x8_d_st7586s_s028hn118a(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  if (u8x8_d_st7586s_common(u8x8, msg, arg_int, arg_ptr) != 0)
    return 1;
  
  switch (msg) {
  case U8X8_MSG_DISPLAY_INIT:
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_st7586s_s028hn118a_init_seq);
    break;
  case U8X8_MSG_DISPLAY_SETUP_MEMORY:
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7586s_s028hn118a_display_info);
    break;
  case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
  	if ( arg_int == 0 )
    {
       u8x8_cad_SendSequence(u8x8, u8x8_d_st7586s_s028hn118a_flip0_seq);
       u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7586s_s028hn118a_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }	
    break;
  default:
    return 0;
  }
  return 1;
}
