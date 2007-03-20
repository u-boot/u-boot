#include <video_logo.h>
#define write_dest_byte(val) {*dest++=val;}
#define BLACK   (0x01800180)	/* black pixel pattern  */
#define BLUE    (0x296E29F0)	/* blue pixel pattern   */
#define RED     (0x51F0515A)	/* red pixel pattern    */
#define MAGENTA (0x6ADE6ACA)	/* magenta pixel pattern */
#define GREEN   (0x91229136)	/* green pixel pattern  */
#define CYAN    (0xAA10AAA6)	/* cyan pixel pattern   */
#define YELLOW  (0xD292D210)	/* yellow pixel pattern */
#define WHITE   (0xFE80FE80)	/* white pixel pattern  */

#define true 	1
#define false	0

typedef struct {
	unsigned int SAV;
	unsigned int EAV;
} SystemCodeType;

const SystemCodeType SystemCodeMap[4] = {
	{0xFF000080, 0xFF00009D},
	{0xFF0000AB, 0xFF0000B6},
	{0xFF0000C7, 0xFF0000DA},
	{0xFF0000EC, 0xFF0000F1}
};
