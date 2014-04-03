#include <stdio.h>
#include <stdlib.h>
#include "membuf.h"
#include "membuf_io.h"
#include "exo_helper.h"

int exo_crunch(char *out_filename, char *buffer, unsigned int len)
{
    int backwards_mode = 0;
    int reverse_mode = 0;
    int output_length = 0;
	
    static struct crunch_options options[1] = { CRUNCH_OPTIONS_DEFAULT };

    struct membuf inbuf[1];
    struct membuf outbuf[1];

	membuf_init(inbuf);
    membuf_init(outbuf);

	membuf_append(inbuf, buffer, len);
	
	struct crunch_info info[1];
	if(backwards_mode)
	{
		crunch_backwards(inbuf, outbuf, options, info);
	}
	else
	{
		crunch(inbuf, outbuf, options, info);
	}
//	LOG(LOG_NORMAL, (" the safety offset is %d.\n",
//					 info->needed_safety_offset));

    if(reverse_mode)
    {
        reverse_buffer(membuf_get(outbuf), membuf_memlen(outbuf));
    }

    write_file(out_filename, outbuf);
    output_length = outbuf->len;
	
    membuf_free(outbuf);
    membuf_free(inbuf);

    return output_length;
}
