###
# scripts contains sources for various helper programs used throughout
# the kernel for the build process.
# ---------------------------------------------------------------------------
# docproc:       Used in Documentation/DocBook

# The following hostprogs-y programs are only build on demand
hostprogs-y += docproc

# These targets are used internally to avoid "is up to date" messages
PHONY += build_docproc
build_docproc: $(obj)/docproc
	@:

# Let clean descend into subdirs
subdir-	+= basic kconfig
