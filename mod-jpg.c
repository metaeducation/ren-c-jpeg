//
//  File: %mod-jpg.c
//  Summary: "JPEG codec natives (dependent on %sys-core.h)"
//  Section: Extension
//  Project: "Rebol 3 Interpreter and Run-time (Ren-C branch)"
//  Homepage: https://github.com/metaeducation/ren-c/
//
//=////////////////////////////////////////////////////////////////////////=//
//
// Copyright 2012 REBOL Technologies
// Copyright 2012-2017 Ren-C Open Source Contributors
// REBOL is a trademark of REBOL Technologies
//
// See README.md and CREDITS.md for more information.
//
// Licensed under the Lesser GPL, Version 3.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// https://www.gnu.org/licenses/lgpl-3.0.html
//
//=////////////////////////////////////////////////////////////////////////=//
//
// The original JPEG encoder and decoder did not include sys-core.h.   But
// after getting rid of the REBCDI-based interface and converting codecs to
// be natives, it's necessary to include the core.
//

#include "tmp-mod-jpg.h"

#include <setjmp.h>
#include <stdbool.h>
#include <stdint.h>
#include <assert.h>
#include <stdlib.h>  // memcpy, etc.

#include "c-enhanced.h"

typedef unsigned char Byte;

// These routines live in %u-jpg.c

extern jmp_buf jpeg_state;
extern void jpeg_info(char *buffer, int nbytes, int *w, int *h);
extern void jpeg_load(char *buffer, int nbytes, char *output);


//
//  identify-jpeg?: native [
//
//  "Codec for identifying BLOB! data for a JPEG"
//
//      return: [logic?]
//      data [blob!]
//  ]
//
DECLARE_NATIVE(IDENTIFY_JPEG_Q)
{
    INCLUDE_PARAMS_OF_IDENTIFY_JPEG_Q;

    // !!! jpeg_info is not const-correct; we trust it not to modify data
    //
    size_t size;
    Byte* data = m_cast(Byte*, rebLockBytes(&size, "data"));

    if (setjmp(jpeg_state)) {  // Handle JPEG error throw
        rebUnlockBytes(data);  // have to call before returning
        return rebLogic(false);
    }

    int w, h;
    jpeg_info(s_cast(data), size, &w, &h);  // may longjmp above
    rebUnlockBytes(data);  // have to call before returning

    return rebLogic(true);
}


//
//  decode-jpeg: native [
//
//  "Codec for decoding BINARY! data for a JPEG"
//
//      return: [fundamental?]  ; image! not currently exposed
//      data [blob!]
//  ]
//
DECLARE_NATIVE(DECODE_JPEG)
{
    INCLUDE_PARAMS_OF_DECODE_JPEG;

    // !!! jpeg code is not const-correct, we trust it not to modify data
    //
    size_t size;
    Byte* data = m_cast(Byte*, rebLockBytes(&size, "data"));

    // Handle JPEG error throw:
    if (setjmp(jpeg_state))
        return "fail -{JPEG decoding failure}-";  // auto-unlocks data on fail

    int w, h;
    jpeg_info(s_cast(data), size, &w, &h); // may longjmp above

    char* image_bytes = rebAllocN(char, (w * h) * 4);  // RGBA is 4 bytes

    jpeg_load(s_cast(data), size, image_bytes);
    rebUnlockBytes(data);  // have to call before returning

    RebolValue* blob = rebRepossess(image_bytes, (w * h) * 4);

    RebolValue* image = rebValue(
        "make-image compose [",
            "(make pair! [", rebI(w), rebI(h), "])",
            blob,
        "]"
    );

    rebRelease(blob);

    return image;
}
