# SPDX-License-Identifier: GPL-2.0+
#
# (C) Copyright 2000-2007
# Wolfgang Denk, DENX Software Engineering, wd@denx.de.

ifdef CONFIG_DM
obj-$(CONFIG_$(PHASE_)BACKLIGHT) += backlight-uclass.o
obj-$(CONFIG_BACKLIGHT_GPIO) += backlight_gpio.o
obj-$(CONFIG_BACKLIGHT_PWM) += pwm_backlight.o
obj-$(CONFIG_$(PHASE_)CONSOLE_NORMAL) += console_normal.o
obj-$(CONFIG_CONSOLE_ROTATION) += console_rotate.o
ifdef CONFIG_$(PHASE_)CONSOLE_NORMAL
obj-y += console_core.o
else ifdef CONFIG_CONSOLE_ROTATION
obj-y += console_core.o
endif
obj-$(CONFIG_CONSOLE_ROTATION) += console_core.o
obj-$(CONFIG_CONSOLE_TRUETYPE) += console_truetype.o fonts/
obj-$(CONFIG_DISPLAY) += display-uclass.o
obj-$(CONFIG_VIDEO_MIPI_DSI) += dsi-host-uclass.o
obj-$(CONFIG_$(PHASE_)VIDEO) += video-uclass.o vidconsole-uclass.o
obj-$(CONFIG_$(PHASE_)VIDEO) += video_bmp.o
obj-$(CONFIG_$(PHASE_)PANEL) += panel-uclass.o
obj-$(CONFIG_PANEL_HX8238D) += hx8238d.o
obj-$(CONFIG_$(PHASE_)SIMPLE_PANEL) += simple_panel.o

obj-$(CONFIG_VIDEO_LOGO) += u_boot_logo.o
obj-$(CONFIG_$(PHASE_)BMP) += bmp.o

endif

obj-$(CONFIG_BACKLIGHT_AAT2870) += aat2870_backlight.o
obj-$(CONFIG_BACKLIGHT_LM3532) += lm3532_backlight.o
obj-$(CONFIG_BACKLIGHT_LM3533) += lm3533_backlight.o
obj-$(CONFIG_BACKLIGHT_LP855x) += lp855x_backlight.o
obj-$(CONFIG_BACKLIGHT_SAMSUNG_CMC623) += cmc623_backlight.o
obj-${CONFIG_EXYNOS_FB} += exynos/
obj-${CONFIG_VIDEO_ROCKCHIP} += rockchip/
obj-${CONFIG_VIDEO_STM32} += stm32/
obj-${CONFIG_$(PHASE_)VIDEO_TIDSS} += tidss/
obj-y += ti/

obj-$(CONFIG_ATMEL_HLCD) += atmel_hlcdfb.o
obj-$(CONFIG_ATMEL_LCD) += atmel_lcdfb.o
obj-$(CONFIG_IHS_VIDEO_OUT) += ihs_video_out.o
obj-$(CONFIG_LOGICORE_DP_TX) += logicore_dp_tx.o
obj-$(CONFIG_NXP_TDA19988) += tda19988.o
obj-$(CONFIG_OSD) += video_osd-uclass.o
obj-$(CONFIG_SANDBOX_OSD) += sandbox_osd.o
obj-$(CONFIG_VIDEO_ARM_MALIDP) += mali_dp.o
obj-$(CONFIG_VIDEO_BCM2835) += bcm2835.o
obj-$(CONFIG_VIDEO_BOCHS) += bochs.o
obj-$(CONFIG_VIDEO_BROADWELL_IGD) += broadwell_igd.o
obj-$(CONFIG_VIDEO_COREBOOT) += coreboot.o
obj-$(CONFIG_VIDEO_DW_HDMI) += dw_hdmi.o
obj-$(CONFIG_VIDEO_DW_MIPI_DSI) += dw_mipi_dsi.o
obj-$(CONFIG_VIDEO_EFI) += efi.o
obj-y += imx/
obj-$(CONFIG_VIDEO_IVYBRIDGE_IGD) += ivybridge_igd.o
obj-$(CONFIG_VIDEO_LCD_ANX9804) += anx9804.o
obj-$(CONFIG_VIDEO_LCD_ENDEAVORU) += endeavoru-panel.o
obj-$(CONFIG_VIDEO_LCD_HIMAX_HX8394) += himax-hx8394.o
obj-$(CONFIG_VIDEO_LCD_HITACHI_TX10D07VM0BAA) += hitachi-tx10d07vm0baa.o
obj-$(CONFIG_VIDEO_LCD_HITACHI_TX18D42VM) += hitachi_tx18d42vm_lcd.o
obj-$(CONFIG_VIDEO_LCD_LG_LD070WX3) += lg-ld070wx3.o
obj-$(CONFIG_VIDEO_LCD_LG_LH400WV3) += lg-lh400wv3-sd04.o
obj-$(CONFIG_VIDEO_LCD_MOT) += mot-panel.o
obj-$(CONFIG_VIDEO_LCD_NOVATEK_NT35510) += novatek-nt35510.o
obj-$(CONFIG_VIDEO_LCD_ORISETECH_OTM8009A) += orisetech_otm8009a.o
obj-$(CONFIG_VIDEO_LCD_RAYDIUM_RM68200) += raydium-rm68200.o
obj-$(CONFIG_VIDEO_LCD_RENESAS_R61307) += renesas-r61307.o
obj-$(CONFIG_VIDEO_LCD_RENESAS_R69328) += renesas-r69328.o
obj-$(CONFIG_VIDEO_LCD_SAMSUNG_LTL106HL02) += samsung-ltl106hl02.o
obj-$(CONFIG_VIDEO_LCD_SHARP_LQ079L1SX01) += sharp-lq079l1sx01.o
obj-$(CONFIG_VIDEO_LCD_SHARP_LQ101R1SX01) += sharp-lq101r1sx01.o
obj-$(CONFIG_VIDEO_LCD_SSD2828) += ssd2828.o
obj-$(CONFIG_VIDEO_LCD_TDO_TL070WSH30) += tdo-tl070wsh30.o
obj-$(CONFIG_VIDEO_LCD_SONY_L4F00430T01) += sony-l4f00430t01.o
obj-$(CONFIG_VIDEO_LCD_SAMSUNG_S6E63M0) += samsung-s6e63m0.o
obj-$(CONFIG_VIDEO_MCDE_SIMPLE) += mcde_simple.o
obj-${CONFIG_VIDEO_MESON} += meson/
obj-${CONFIG_VIDEO_MIPI_DSI} += mipi_dsi.o
obj-$(CONFIG_VIDEO_MVEBU) += mvebu_lcd.o
obj-$(CONFIG_VIDEO_MXS) += mxsfb.o videomodes.o
obj-$(CONFIG_VIDEO_NX) += nexell_display.o videomodes.o nexell/
obj-$(CONFIG_VIDEO_OMAP3) += omap3_dss.o
obj-$(CONFIG_VIDEO_DSI_HOST_SANDBOX) += sandbox_dsi_host.o
obj-$(CONFIG_VIDEO_SANDBOX_SDL) += sandbox_sdl.o
obj-$(CONFIG_VIDEO_SIMPLE) += simplefb.o
obj-$(CONFIG_VIDEO_VESA) += vesa.o
obj-$(CONFIG_VIDEO_SEPS525) += seps525.o
obj-$(CONFIG_VIDEO_ZYNQMP_DPSUB) += zynqmp/

obj-y += bridge/
obj-y += sunxi/
obj-y += tegra/
