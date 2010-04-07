
/******************************************************************************
*
* $xilinx_legal_disclaimer
*
******************************************************************************/

#include "pl353_x8.h"
/*
 * init_nor_flash init the parameters of pl353 for the P30 flash
 */
//typedef volatile u32 XIo_Address;

void Xil_Out8(XIo_Address OutAddress, u8 Value);
u8 Xil_In8(XIo_Address InAddress);

void init_nor_flash(void)
{
  /* Init variables */

   /* Write timing info to set_cycles registers */
  u32 set_cycles_reg = (0x0 << 20) | /* Set_t6 or we_time from sram_cycles */
  		       (0x0 << 17) |   /* Set_t5 or t_tr from sram_cycles */
		       (0x1 << 14) | /* Set_t4 or t_pc from sram_cycles */
		       (0x5 << 11) | /* Set_t3 or t_wp from sram_cycles */
		       (0x1 << 8) |  /* Set_t2 t_ceoe from sram_cycles */
		       (0x7 << 4) |  /* Set_t1 t_wc from sram_cycles */
		       (0x7);	     /* Set_t0 t_rc from sram_cycles */

  X_mWriteReg(PARPORT_CRTL_BASEADDR, PARPORT_MC_SET_CYCLES, set_cycles_reg);

  /* write operation mode to set_opmode registers */
  u32 set_opmode_reg = (0x1 << 13) | /* set_burst_align, see to 32 beats */
  		       (0x0 << 12) | /* set_bls, set to default I am not sure */
		       (0x0 << 11) | /* set_adv bit, set to default */
		       (0x0 << 10) | /* set_baa, I guess we don't use baa_n */
 		       (0x0 << 7) |  /* set_wr_bl, write brust length, set to 0 */
                       (0x0 << 6) |  /* set_wr_sync, set to 0 */
 		       (0x0 << 3) |  /* set_rd_bl, read brust lenght, set to 0 */
 		       (0x0 << 2) |  /* set_rd_sync, set to 0 */
 		       (0x0 );       /* set_mw, memory width, 16bits width*/
  X_mWriteReg(PARPORT_CRTL_BASEADDR, PARPORT_MC_SET_OPMODE, set_opmode_reg);


  /*
   * Issue a direct_cmd by writing to direct_cmd register
   * This is needed becuase the UpdatesReg flag in direct_cmd updates the state of SMC
   * I think....
   */
  u32 direct_cmd_reg = (0x0 << 23) | /* chip 0 from interface 0 */
                       (0x2 << 21) | /* UpdateRegs operation, to update the two reg we wrote earlier*/
		       (0x0 << 20) | /* Not sure about this one cre, what does it do? */
		       (0x0);        /* addr, not use in UpdateRegs */
  X_mWriteReg(PARPORT_CRTL_BASEADDR, PARPORT_MC_DIRECT_CMD, direct_cmd_reg);

  /* Now the flash should be ready to be accessed */
}


/*
 * wirte_nor_flash returns 1 after a seccussful write
 */
void write_half_word_nor_flash(u32 address, u8 data)
{
  /* status reg polling to be added later */
  Xil_Out8(address, 0x40);
  Xil_Out8(address, data);
  while(read_status_reg_nor_flash(address) >> 7 != 1);
}

void write_byte_nor_flash(u32 address, u8 data)
{
  Xil_Out8(address, 0x40);
  Xil_Out8(address, data);
  while(read_status_reg_nor_flash(address) >> 7 != 1);
}
/*
 * read_nor_flash returns the data of a memeory location
 */
u16  read_half_word_nor_flash(u32 address)
{
  Xil_Out8(address, 0xFF);
  return Xil_In8(address);
}

/*
 * read_status_reg_nor_flash returns the status register of a block address
 */
u16  read_status_reg_nor_flash(u32 address)
{
  Xil_Out8(address, 0x70);
  return Xil_In8(address);
}

/*
 * clear_status_reg_nor_flash clears the status register of a block address
 */
void clear_status_reg_nor_flash(u32 address)
{
  Xil_Out8(address, 0x50);
}

/*
 * unlock_nor_flash put the selected block of address in unlock mode
 */
void unlock_nor_flash(u32 blockAddress)
{
  Xil_Out8((blockAddress & 0xFFFF0000), 0x60);
  Xil_Out8((blockAddress & 0xFFFF0000), 0xD0);
}

/*
 * lock_nor_flash put the selected block of address in lock mode
 */
void lock_nor_flash(u32 blockAddress)
{
  Xil_Out8((blockAddress & 0xFFFF0000), 0x60);
  Xil_Out8((blockAddress & 0xFFFF0000), 0x01);
}

/*
 * block_erase_nor_flash
 */
void block_erase_nor_flash(u32 blockAddress)
{
  /* Clear status before erase */
  if(read_status_reg_nor_flash(blockAddress & 0xFFFF0000) != 0x80)
  {
     clear_status_reg_nor_flash(blockAddress & 0xFFFF0000);
  }

  Xil_Out8((blockAddress & 0xFFFF0000), 0x20);
  Xil_Out8((blockAddress & 0xFFFF0000), 0xD0);

  while(Xil_In8((blockAddress & 0xFFFF0000)) >> 7 != 1);

}

/*
 * buffered_write_nor_flash
 */
void buffered_wirte_nor_flash(u32 address, u8 *dataBuffer, u16 wordCount)
{

  /* Issue write program */
  Xil_Out8((address & 0xFFFF0000), 0xE8);

  /* Poll status reg at block address until ready*/
  while(Xil_In8((address & 0xFFFF0000)) >> 7 != 1);

  /* write count to block address */
  Xil_Out8((address & 0xFFFF0000), wordCount - 1);

  /* write data */
  int i;
  for(i = 0; i < wordCount; i++)
  {
     Xil_Out8((address + i), dataBuffer[i]);
  }

  /* write confirm to block address  */
  Xil_Out8((address & 0xFFFF0000), 0xD0);

  /* Poll status reg */
  while(Xil_In8((address & 0xFFFF0000)) >> 7 != 1);


}
void Xil_Out8(XIo_Address OutAddress, u8 Value)
{
    *(volatile u8 *) OutAddress = Value;
}

u8 Xil_In8(XIo_Address InAddress)
{
    return *(u8 *) InAddress;
}
