/* Debugging */

Scope(\) {
    /* Debug Output */
    OperationRegion(DBG, SystemIO, 0x0402, 0x01)
    Field(DBG, ByteAcc, NoLock, Preserve) {
        DBGB,   8,
    }
	/*
	 * Debug method - use this method to send output to the QEMU
	 * BIOS debug port.  This method handles strings, integers,
	 * and buffers.  For example: DBUG("abc") DBUG(0x123)
	 */
    Method(DBUG, 1) {
        ToHexString(Arg0, Local0)
        ToBuffer(Local0, Local0)
        Subtract(SizeOf(Local0), 1, Local1)
        Store(Zero, Local2)
        While (LLess(Local2, Local1)) {
            Store(DerefOf(Index(Local0, Local2)), DBGB)
            Increment(Local2)
        }
        Store(0x0a, dbgb)
    }
}
