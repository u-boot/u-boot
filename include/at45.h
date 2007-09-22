
#ifndef	_AT45_H_
#define	_AT45_H_
#ifdef	DATAFLASH_MMC_SELECT
extern	void AT91F_SelectMMC(void);
extern	void AT91F_SelectSPI(void);
extern	int AT91F_GetMuxStatus(void);
#endif
extern	void AT91F_SpiInit(void);
extern	void AT91F_SpiEnable(int cs);
extern	unsigned int AT91F_SpiWrite ( AT91PS_DataflashDesc pDesc );
extern	AT91S_DataFlashStatus AT91F_DataFlashSendCommand(
		AT91PS_DataFlash pDataFlash,
		unsigned char OpCode,
		unsigned int CmdSize,
		unsigned int DataflashAddress);
extern	AT91S_DataFlashStatus AT91F_DataFlashGetStatus (
	AT91PS_DataflashDesc pDesc);
extern	AT91S_DataFlashStatus AT91F_DataFlashWaitReady (
	AT91PS_DataflashDesc pDataFlashDesc,
	unsigned int timeout);
extern	AT91S_DataFlashStatus AT91F_DataFlashContinuousRead (
	AT91PS_DataFlash pDataFlash,
	int src,
	unsigned char *dataBuffer,
	int sizeToRead );
extern	AT91S_DataFlashStatus AT91F_DataFlashPagePgmBuf(
	AT91PS_DataFlash pDataFlash,
	unsigned char *src,
	unsigned int dest,
	unsigned int SizeToWrite);
extern	AT91S_DataFlashStatus AT91F_MainMemoryToBufferTransfert(
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned int page);
extern	AT91S_DataFlashStatus AT91F_DataFlashWriteBuffer (
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned char *dataBuffer,
	unsigned int bufferAddress,
	int SizeToWrite );
extern	AT91S_DataFlashStatus AT91F_PageErase(
	AT91PS_DataFlash pDataFlash,
	unsigned int page);
extern	AT91S_DataFlashStatus AT91F_BlockErase(
	AT91PS_DataFlash pDataFlash,
	unsigned int block);
extern	AT91S_DataFlashStatus AT91F_WriteBufferToMain (
	AT91PS_DataFlash pDataFlash,
	unsigned char BufferCommand,
	unsigned int dest );
extern	AT91S_DataFlashStatus AT91F_PartialPageWrite (
	AT91PS_DataFlash pDataFlash,
	unsigned char *src,
	unsigned int dest,
	unsigned int size);
extern	AT91S_DataFlashStatus AT91F_DataFlashWrite(
	AT91PS_DataFlash pDataFlash,
	unsigned char *src,
	int dest,
	int size );
extern	int AT91F_DataFlashRead(
	AT91PS_DataFlash pDataFlash,
	unsigned long addr,
	unsigned long size,
	char *buffer);
extern	int AT91F_DataflashProbe(int cs, AT91PS_DataflashDesc pDesc);

#endif
