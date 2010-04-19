#!/usr/bin/gawk -f
BEGIN {
	print "/* DO NOT EDIT: AUTOMATICALLY GENERATED"
	print " * Input files: bootrom-asm-offsets.awk bootrom-asm-offsets.c.in"
	print " * DO NOT EDIT: AUTOMATICALLY GENERATED"
	print " */"
	print ""
	system("cat bootrom-asm-offsets.c.in")
	print "{"
}

{
	/* find a structure definition */
	if ($0 ~ /typedef struct .* {/) {
		delete members;
		i = 0;

		/* extract each member of the structure */
		while (1) {
			getline
			if ($1 == "}")
				break;
			gsub(/[*;]/, "");
			members[i++] = $NF;
		}

		/* grab the structure's name */
		struct = $NF;
		sub(/;$/, "", struct);

		/* output the DEFINE() macros */
		while (i-- > 0)
			print "\tDEFINE(" struct ", " members[i] ");"
		print ""
	}
}

END {
	print "\treturn 0;"
	print "}"
}
