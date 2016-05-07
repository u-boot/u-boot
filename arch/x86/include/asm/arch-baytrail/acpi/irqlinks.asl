/*
 * Copyright (C) 2007-2009 coresystems GmbH
 * Copyright (C) 2016 Bin Meng <bmeng.cn@gmail.com>
 *
 * Modified from coreboot src/soc/intel/baytrail/acpi/irqlinks.asl
 *
 * SPDX-License-Identifier:	GPL-2.0+
 */

Scope (\)
{
	/* Intel Legacy Block */
	OperationRegion(ILBS, SystemMemory, ILB_BASE_ADDRESS, ILB_BASE_SIZE)
	Field(ILBS, AnyAcc, NoLock, Preserve) {
		Offset (0x8),
		PRTA, 8,
		PRTB, 8,
		PRTC, 8,
		PRTD, 8,
		PRTE, 8,
		PRTF, 8,
		PRTG, 8,
		PRTH, 8,
	}
}

Device (LNKA)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 1)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTA)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLA, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLA, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTA */
		ShiftLeft(1, And(PRTA, 0x0f), IRQ0)

		Return (RTLA)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTA)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTA, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKB)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 2)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTB)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLB, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLB, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTB */
		ShiftLeft(1, And(PRTB, 0x0f), IRQ0)

		Return (RTLB)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTB)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTB, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKC)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 3)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTC)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLC, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLC, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTC */
		ShiftLeft(1, And(PRTC, 0x0f), IRQ0)

		Return (RTLC)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTC)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTC, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKD)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 4)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTD)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLD, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLD, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTD */
		ShiftLeft(1, And(PRTD, 0x0f), IRQ0)

		Return (RTLD)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTD)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTD, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKE)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 5)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTE)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLE, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLE, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTE */
		ShiftLeft(1, And(PRTE, 0x0f), IRQ0)

		Return (RTLE)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTE)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTE, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKF)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 6)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTF)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLF, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLF, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTF */
		ShiftLeft(1, And(PRTF, 0x0f), IRQ0)

		Return (RTLF)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTF)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTF, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKG)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 7)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTG)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLG, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLG, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTG */
		ShiftLeft(1, And(PRTG, 0x0f), IRQ0)

		Return (RTLG)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTG)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTG, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}

Device (LNKH)
{
	Name(_HID, EISAID("PNP0C0F"))
	Name(_UID, 8)

	/* Disable method */
	Method(_DIS, 0, Serialized)
	{
		Store(0x80, PRTH)
	}

	/* Possible Resource Settings for this Link */
	Name(_PRS, ResourceTemplate()
	{
		IRQ(Level, ActiveLow, Shared) { 5, 6, 7, 10, 11, 12, 14, 15 }
	})

	/* Current Resource Settings for this link */
	Method(_CRS, 0, Serialized)
	{
		Name(RTLH, ResourceTemplate()
		{
			IRQ(Level, ActiveLow, Shared) {}
		})
		CreateWordField(RTLH, 1, IRQ0)

		/* Clear the WordField */
		Store(Zero, IRQ0)

		/* Set the bit from PRTH */
		ShiftLeft(1, And(PRTH, 0x0f), IRQ0)

		Return (RTLH)
	}

	/* Set Resource Setting for this IRQ link */
	Method(_SRS, 1, Serialized)
	{
		CreateWordField(Arg0, 1, IRQ0)

		/* Which bit is set? */
		FindSetRightBit(IRQ0, Local0)

		Decrement(Local0)
		Store(Local0, PRTH)
	}

	/* Status */
	Method(_STA, 0, Serialized)
	{
		If (And(PRTH, 0x80)) {
			Return (STA_DISABLED)
		} Else {
			Return (STA_INVISIBLE)
		}
	}
}
