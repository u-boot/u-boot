/*
 * USB Masks
 */

#ifndef __BFIN_PERIPHERAL_USB__
#define __BFIN_PERIPHERAL_USB__

/* Bit masks for USB_FADDR */

#define FUNCTION_ADDRESS	0x7f	/* Function address */

/* Bit masks for USB_POWER */

#define ENABLE_SUSPENDM		0x1	/* enable SuspendM output */
#define SUSPEND_MODE		0x2	/* Suspend Mode indicator */
#define RESUME_MODE		0x4	/* DMA Mode */
#define RESET			0x8	/* Reset indicator */
#define HS_MODE			0x10	/* High Speed mode indicator */
#define HS_ENABLE		0x20	/* high Speed Enable */
#define SOFT_CONN		0x40	/* Soft connect */
#define ISO_UPDATE		0x80	/* Isochronous update */

/* Bit masks for USB_INTRTX */

#define EP0_TX			0x1	/* Tx Endpoint 0 interrupt */
#define EP1_TX			0x2	/* Tx Endpoint 1 interrupt */
#define EP2_TX			0x4	/* Tx Endpoint 2 interrupt */
#define EP3_TX			0x8	/* Tx Endpoint 3 interrupt */
#define EP4_TX			0x10	/* Tx Endpoint 4 interrupt */
#define EP5_TX			0x20	/* Tx Endpoint 5 interrupt */
#define EP6_TX			0x40	/* Tx Endpoint 6 interrupt */
#define EP7_TX			0x80	/* Tx Endpoint 7 interrupt */

/* Bit masks for USB_INTRRX */

#define EP1_RX			0x2	/* Rx Endpoint 1 interrupt */
#define EP2_RX			0x4	/* Rx Endpoint 2 interrupt */
#define EP3_RX			0x8	/* Rx Endpoint 3 interrupt */
#define EP4_RX			0x10	/* Rx Endpoint 4 interrupt */
#define EP5_RX			0x20	/* Rx Endpoint 5 interrupt */
#define EP6_RX			0x40	/* Rx Endpoint 6 interrupt */
#define EP7_RX			0x80	/* Rx Endpoint 7 interrupt */

/* Bit masks for USB_INTRTXE */

#define EP0_TX_E		0x1	/* Endpoint 0 interrupt Enable */
#define EP1_TX_E		0x2	/* Tx Endpoint 1 interrupt enable */
#define EP2_TX_E		0x4	/* Tx Endpoint 2 interrupt enable */
#define EP3_TX_E		0x8	/* Tx Endpoint 3 interrupt enable */
#define EP4_TX_E		0x10	/* Tx Endpoint 4 interrupt enable */
#define EP5_TX_E		0x20	/* Tx Endpoint 5 interrupt enable */
#define EP6_TX_E		0x40	/* Tx Endpoint 6 interrupt enable */
#define EP7_TX_E		0x80	/* Tx Endpoint 7 interrupt enable */

/* Bit masks for USB_INTRRXE */

#define EP1_RX_E		0x02	/* Rx Endpoint 1 interrupt enable */
#define EP2_RX_E		0x04	/* Rx Endpoint 2 interrupt enable */
#define EP3_RX_E		0x08	/* Rx Endpoint 3 interrupt enable */
#define EP4_RX_E		0x10	/* Rx Endpoint 4 interrupt enable */
#define EP5_RX_E		0x20	/* Rx Endpoint 5 interrupt enable */
#define EP6_RX_E		0x40	/* Rx Endpoint 6 interrupt enable */
#define EP7_RX_E		0x80	/* Rx Endpoint 7 interrupt enable */

/* Bit masks for USB_INTRUSB */

#define SUSPEND_B		0x01	/* Suspend indicator */
#define RESUME_B		0x02	/* Resume indicator */
#define RESET_OR_BABLE_B	0x04	/* Reset/babble indicator */
#define SOF_B			0x08	/* Start of frame */
#define CONN_B			0x10	/* Connection indicator */
#define DISCON_B		0x20	/* Disconnect indicator */
#define SESSION_REQ_B		0x40	/* Session Request */
#define VBUS_ERROR_B		0x80	/* Vbus threshold indicator */

/* Bit masks for USB_INTRUSBE */

#define SUSPEND_BE		0x01	/* Suspend indicator int enable */
#define RESUME_BE		0x02	/* Resume indicator int enable */
#define RESET_OR_BABLE_BE	0x04	/* Reset/babble indicator int enable */
#define SOF_BE			0x08	/* Start of frame int enable */
#define CONN_BE			0x10	/* Connection indicator int enable */
#define DISCON_BE		0x20	/* Disconnect indicator int enable */
#define SESSION_REQ_BE		0x40	/* Session Request int enable */
#define VBUS_ERROR_BE		0x80	/* Vbus threshold indicator int enable */

/* Bit masks for USB_FRAME */

#define FRAME_NUMBER		0x7ff	/* Frame number */

/* Bit masks for USB_INDEX */

#define SELECTED_ENDPOINT	0xf	/* selected endpoint */

/* Bit masks for USB_GLOBAL_CTL */

#define GLOBAL_ENA		0x0001	/* enables USB module */
#define EP1_TX_ENA		0x0002	/* Transmit endpoint 1 enable */
#define EP2_TX_ENA		0x0004	/* Transmit endpoint 2 enable */
#define EP3_TX_ENA		0x0008	/* Transmit endpoint 3 enable */
#define EP4_TX_ENA		0x0010	/* Transmit endpoint 4 enable */
#define EP5_TX_ENA		0x0020	/* Transmit endpoint 5 enable */
#define EP6_TX_ENA		0x0040	/* Transmit endpoint 6 enable */
#define EP7_TX_ENA		0x0080	/* Transmit endpoint 7 enable */
#define EP1_RX_ENA		0x0100	/* Receive endpoint 1 enable */
#define EP2_RX_ENA		0x0200	/* Receive endpoint 2 enable */
#define EP3_RX_ENA		0x0400	/* Receive endpoint 3 enable */
#define EP4_RX_ENA		0x0800	/* Receive endpoint 4 enable */
#define EP5_RX_ENA		0x1000	/* Receive endpoint 5 enable */
#define EP6_RX_ENA		0x2000	/* Receive endpoint 6 enable */
#define EP7_RX_ENA		0x4000	/* Receive endpoint 7 enable */

/* Bit masks for USB_OTG_DEV_CTL */

#define SESSION			0x1	/* session indicator */
#define HOST_REQ		0x2	/* Host negotiation request */
#define HOST_MODE		0x4	/* indicates USBDRC is a host */
#define VBUS0			0x8	/* Vbus level indicator[0] */
#define VBUS1			0x10	/* Vbus level indicator[1] */
#define LSDEV			0x20	/* Low-speed indicator */
#define FSDEV			0x40	/* Full or High-speed indicator */
#define B_DEVICE		0x80	/* A' or 'B' device indicator */

/* Bit masks for USB_OTG_VBUS_IRQ */

#define DRIVE_VBUS_ON		0x1	/* indicator to drive VBUS control circuit */
#define DRIVE_VBUS_OFF		0x2	/* indicator to shut off charge pump */
#define CHRG_VBUS_START		0x4	/* indicator for external circuit to start charging VBUS */
#define CHRG_VBUS_END		0x8	/* indicator for external circuit to end charging VBUS */
#define DISCHRG_VBUS_START	0x10	/* indicator to start discharging VBUS */
#define DISCHRG_VBUS_END	0x20	/* indicator to stop discharging VBUS */

/* Bit masks for USB_OTG_VBUS_MASK */

#define DRIVE_VBUS_ON_ENA	0x01	/* enable DRIVE_VBUS_ON interrupt */
#define DRIVE_VBUS_OFF_ENA	0x02	/* enable DRIVE_VBUS_OFF interrupt */
#define CHRG_VBUS_START_ENA	0x04	/* enable CHRG_VBUS_START interrupt */
#define CHRG_VBUS_END_ENA	0x08	/* enable CHRG_VBUS_END interrupt */
#define DISCHRG_VBUS_START_ENA	0x10	/* enable DISCHRG_VBUS_START interrupt */
#define DISCHRG_VBUS_END_ENA	0x20	/* enable DISCHRG_VBUS_END interrupt */

/* Bit masks for USB_CSR0 */

#define RXPKTRDY		0x1	/* data packet receive indicator */
#define TXPKTRDY		0x2	/* data packet in FIFO indicator */
#define STALL_SENT		0x4	/* STALL handshake sent */
#define DATAEND			0x8	/* Data end indicator */
#define SETUPEND		0x10	/* Setup end */
#define SENDSTALL		0x20	/* Send STALL handshake */
#define SERVICED_RXPKTRDY	0x40	/* used to clear the RxPktRdy bit */
#define SERVICED_SETUPEND	0x80	/* used to clear the SetupEnd bit */
#define FLUSHFIFO		0x100	/* flush endpoint FIFO */
#define STALL_RECEIVED_H	0x4	/* STALL handshake received host mode */
#define SETUPPKT_H		0x8	/* send Setup token host mode */
#define ERROR_H			0x10	/* timeout error indicator host mode */
#define REQPKT_H		0x20	/* Request an IN transaction host mode */
#define STATUSPKT_H		0x40	/* Status stage transaction host mode */
#define NAK_TIMEOUT_H		0x80	/* EP0 halted after a NAK host mode */

/* Bit masks for USB_COUNT0 */

#define EP0_RX_COUNT		0x7f	/* number of received bytes in EP0 FIFO */

/* Bit masks for USB_NAKLIMIT0 */

#define EP0_NAK_LIMIT		0x1f	/* frames/micro frames count after which EP0 timeouts */

/* Bit masks for USB_TX_MAX_PACKET */

#define MAX_PACKET_SIZE_T	0x7ff	/* maximum data pay load in a frame */

/* Bit masks for USB_RX_MAX_PACKET */

#define MAX_PACKET_SIZE_R	0x7ff	/* maximum data pay load in a frame */

/* Bit masks for USB_TXCSR */

#define TXPKTRDY_T		0x1	/* data packet in FIFO indicator */
#define FIFO_NOT_EMPTY_T	0x2	/* FIFO not empty */
#define UNDERRUN_T		0x4	/* TxPktRdy not set for an IN token */
#define FLUSHFIFO_T		0x8	/* flush endpoint FIFO */
#define STALL_SEND_T		0x10	/* issue a Stall handshake */
#define STALL_SENT_T		0x20	/* Stall handshake transmitted */
#define CLEAR_DATATOGGLE_T	0x40	/* clear endpoint data toggle */
#define INCOMPTX_T		0x80	/* indicates that a large packet is split */
#define DMAREQMODE_T		0x400	/* DMA mode (0 or 1) selection */
#define FORCE_DATATOGGLE_T	0x800	/* Force data toggle */
#define DMAREQ_ENA_T		0x1000	/* Enable DMA request for Tx EP */
#define ISO_T			0x4000	/* enable Isochronous transfers */
#define AUTOSET_T		0x8000	/* allows TxPktRdy to be set automatically */
#define ERROR_TH		0x4	/* error condition host mode */
#define STALL_RECEIVED_TH	0x20	/* Stall handshake received host mode */
#define NAK_TIMEOUT_TH		0x80	/* NAK timeout host mode */

/* Bit masks for USB_TXCOUNT */

#define TX_COUNT		0x1fff	/* Byte len for the selected endpoint Tx FIFO */

/* Bit masks for USB_RXCSR */

#define RXPKTRDY_R		0x1	/* data packet in FIFO indicator */
#define FIFO_FULL_R		0x2	/* FIFO not empty */
#define OVERRUN_R		0x4	/* TxPktRdy not set for an IN token */
#define DATAERROR_R		0x8	/* Out packet cannot be loaded into Rx FIFO */
#define FLUSHFIFO_R		0x10	/* flush endpoint FIFO */
#define STALL_SEND_R		0x20	/* issue a Stall handshake */
#define STALL_SENT_R		0x40	/* Stall handshake transmitted */
#define CLEAR_DATATOGGLE_R	0x80	/* clear endpoint data toggle */
#define INCOMPRX_R		0x100	/* indicates that a large packet is split */
#define DMAREQMODE_R		0x800	/* DMA mode (0 or 1) selection */
#define DISNYET_R		0x1000	/* disable Nyet handshakes */
#define DMAREQ_ENA_R		0x2000	/* Enable DMA request for Tx EP */
#define ISO_R			0x4000	/* enable Isochronous transfers */
#define AUTOCLEAR_R		0x8000	/* allows TxPktRdy to be set automatically */
#define ERROR_RH		0x4	/* TxPktRdy not set for an IN token host mode */
#define REQPKT_RH		0x20	/* request an IN transaction host mode */
#define STALL_RECEIVED_RH	0x40	/* Stall handshake received host mode */
#define INCOMPRX_RH		0x100	/* large packet is split host mode */
#define DMAREQMODE_RH		0x800	/* DMA mode (0 or 1) selection host mode */
#define AUTOREQ_RH		0x4000	/* sets ReqPkt automatically host mode */

/* Bit masks for USB_RXCOUNT */

#define RX_COUNT		0x1fff	/* Packet byte len in the Rx FIFO */

/* Bit masks for USB_TXTYPE */

#define TARGET_EP_NO_T		0xf	/* EP number */
#define PROTOCOL_T		0xc	/* transfer type */

/* Bit masks for USB_TXINTERVAL */

#define TX_POLL_INTERVAL	0xff	/* polling interval for selected Tx EP */

/* Bit masks for USB_RXTYPE */

#define TARGET_EP_NO_R		0xf	/* EP number */
#define PROTOCOL_R		0xc	/* transfer type */

/* Bit masks for USB_RXINTERVAL */

#define RX_POLL_INTERVAL	0xff	/* polling interval for selected Rx EP */

/* Bit masks for USB_DMA_INTERRUPT */

#define DMA0_INT		0x1	/* DMA0 pending interrupt */
#define DMA1_INT		0x2	/* DMA1 pending interrupt */
#define DMA2_INT		0x4	/* DMA2 pending interrupt */
#define DMA3_INT		0x8	/* DMA3 pending interrupt */
#define DMA4_INT		0x10	/* DMA4 pending interrupt */
#define DMA5_INT		0x20	/* DMA5 pending interrupt */
#define DMA6_INT		0x40	/* DMA6 pending interrupt */
#define DMA7_INT		0x80	/* DMA7 pending interrupt */

/* Bit masks for USB_DMAxCONTROL */

#define DMA_ENA			0x1	/* DMA enable */
#define DIRECTION		0x2	/* direction of DMA transfer */
#define MODE			0x4	/* DMA Bus error */
#define INT_ENA			0x8	/* Interrupt enable */
#define EPNUM			0xf0	/* EP number */
#define BUSERROR		0x100	/* DMA Bus error */

#endif
