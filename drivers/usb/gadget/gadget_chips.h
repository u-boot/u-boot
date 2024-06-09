/*
 * USB device controllers have lots of quirks.  Use these macros in
 * gadget drivers or other code that needs to deal with them, and which
 * autoconfigures instead of using early binding to the hardware.
 *
 * This SHOULD eventually work like the ARM mach_is_*() stuff, driven by
 * some config file that gets updated as new hardware is supported.
 * (And avoiding all runtime comparisons in typical one-choice configs!)
 *
 * NOTE:  some of these controller drivers may not be available yet.
 * Some are available on 2.4 kernels; several are available, but not
 * yet pushed in the 2.6 mainline tree.
 *
 * Ported to U-Boot by: Thomas Smits <ts.smits@gmail.com> and
 *                      Remy Bohmer <linux@bohmer.net>
 */
#ifdef CONFIG_USB_GADGET_NET2280
#define	gadget_is_net2280(g)	(!strcmp("net2280", (g)->name))
#else
#define	gadget_is_net2280(g)	0
#endif

#ifdef CONFIG_USB_GADGET_AMD5536UDC
#define	gadget_is_amd5536udc(g)	(!strcmp("amd5536udc", (g)->name))
#else
#define	gadget_is_amd5536udc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_DUMMY_HCD
#define	gadget_is_dummy(g)	(!strcmp("dummy_udc", (g)->name))
#else
#define	gadget_is_dummy(g)	0
#endif

#ifdef CONFIG_USB_GADGET_GOKU
#define	gadget_is_goku(g)	(!strcmp("goku_udc", (g)->name))
#else
#define	gadget_is_goku(g)	0
#endif

/* SH3 UDC -- not yet ported 2.4 --> 2.6 */
#ifdef CONFIG_USB_GADGET_SUPERH
#define	gadget_is_sh(g)		(!strcmp("sh_udc", (g)->name))
#else
#define	gadget_is_sh(g)		0
#endif

/* handhelds.org tree (?) */
#ifdef CONFIG_USB_GADGET_MQ11XX
#define	gadget_is_mq11xx(g)	(!strcmp("mq11xx_udc", (g)->name))
#else
#define	gadget_is_mq11xx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_OMAP
#define	gadget_is_omap(g)	(!strcmp("omap_udc", (g)->name))
#else
#define	gadget_is_omap(g)	0
#endif

/* not yet ported 2.4 --> 2.6 */
#ifdef CONFIG_USB_GADGET_N9604
#define	gadget_is_n9604(g)	(!strcmp("n9604_udc", (g)->name))
#else
#define	gadget_is_n9604(g)	0
#endif

#ifdef CONFIG_USB_GADGET_ATMEL_USBA
#define gadget_is_atmel_usba(g)	(!strcmp("atmel_usba_udc", (g)->name))
#else
#define gadget_is_atmel_usba(g)	0
#endif

#ifdef CONFIG_USB_GADGET_AT91
#define gadget_is_at91(g)	(!strcmp("at91_udc", (g)->name))
#else
#define gadget_is_at91(g)	0
#endif

/* status unclear */
#ifdef CONFIG_USB_GADGET_IMX
#define gadget_is_imx(g)	(!strcmp("imx_udc", (g)->name))
#else
#define gadget_is_imx(g)	0
#endif

#ifdef CONFIG_USB_GADGET_FSL_USB2
#define gadget_is_fsl_usb2(g)	(!strcmp("fsl-usb2-udc", (g)->name))
#else
#define gadget_is_fsl_usb2(g)	0
#endif

/* Mentor high speed function controller */
/* from Montavista kernel (?) */
#ifdef CONFIG_USB_GADGET_MUSBHSFC
#define gadget_is_musbhsfc(g)	(!strcmp("musbhsfc_udc", (g)->name))
#else
#define gadget_is_musbhsfc(g)	0
#endif

/* Mentor high speed "dual role" controller, in peripheral role */
#ifdef CONFIG_USB_MUSB_GADGET
#define gadget_is_musbhdrc(g)	(!strcmp("musb-hdrc", (g)->name))
#else
#define gadget_is_musbhdrc(g)	0
#endif

#ifdef CONFIG_USB_GADGET_M66592
#define	gadget_is_m66592(g)	(!strcmp("m66592_udc", (g)->name))
#else
#define	gadget_is_m66592(g)	0
#endif

#ifdef CONFIG_CI_UDC
#define gadget_is_ci(g)        (!strcmp("ci_udc", (g)->name))
#else
#define gadget_is_ci(g)        0
#endif

#ifdef CONFIG_USB_DWC3_GADGET
#define gadget_is_dwc3(g)        (!strcmp("dwc3-gadget", (g)->name))
#else
#define gadget_is_dwc3(g)        0
#endif

#ifdef CONFIG_USB_CDNS3_GADGET
#define gadget_is_cdns3(g)        (!strcmp("cdns3-gadget", (g)->name))
#else
#define gadget_is_cdns3(g)        0
#endif

#ifdef CONFIG_USB_GADGET_MAX3420
#define gadget_is_max3420(g)        (!strcmp("max3420-udc", (g)->name))
#else
#define gadget_is_max3420(g)        0
#endif

#ifdef CONFIG_USB_MTU3_GADGET
#define gadget_is_mtu3(g)        (!strcmp("mtu3-gadget", (g)->name))
#else
#define gadget_is_mtu3(g)        0
#endif

#ifdef CONFIG_USB_GADGET_DWC2_OTG
#define gadget_is_dwc2(g)        (!strcmp("dwc2-udc", (g)->name))
#else
#define gadget_is_dwc2(g)        0
#endif
