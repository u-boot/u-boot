#define BITS_PER_UNIT	8

typedef 	 int HItype		__attribute__ ((mode (HI)));
typedef unsigned int UHItype		__attribute__ ((mode (HI)));

typedef		 int SItype		__attribute__ ((mode (SI)));
typedef unsigned int USItype		__attribute__ ((mode (SI)));

typedef int word_type			__attribute__ ((mode (__word__)));

struct SIstruct {HItype low, high;};

typedef union {
	struct SIstruct s;
	SItype ll;
} SIunion;
