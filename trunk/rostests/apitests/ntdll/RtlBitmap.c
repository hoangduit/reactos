
#define WIN32_NO_STATUS
#include <wine/test.h>
#include <ndk/rtlfuncs.h>
#include <pseh/pseh2.h>

void
Test_RtlFindMostSignificantBit(void)
{
    ok_int(RtlFindMostSignificantBit(0), -1);

    ok_int(RtlFindMostSignificantBit(0x0000000000000001ULL), 0);
    ok_int(RtlFindMostSignificantBit(0x0000000000000002ULL), 1);
    ok_int(RtlFindMostSignificantBit(0x1000000000000000ULL), 60);
    ok_int(RtlFindMostSignificantBit(0x8000000000000000ULL), 63);
    ok_int(RtlFindMostSignificantBit(0x8000000000000001ULL), 63);
    ok_int(RtlFindMostSignificantBit(0xFFFFFFFFFFFFFFFFULL), 63);
    ok_int(RtlFindMostSignificantBit(0x0000000070000000ULL), 30);
}

void
Test_RtlFindLeastSignificantBit(void)
{
    ok_int(RtlFindLeastSignificantBit(0), -1);

    ok_int(RtlFindLeastSignificantBit(0x0000000000000001ULL), 0);
    ok_int(RtlFindLeastSignificantBit(0x0000000000000002ULL), 1);
    ok_int(RtlFindLeastSignificantBit(0x1000000000000000ULL), 60);
    ok_int(RtlFindLeastSignificantBit(0x8000000000000000ULL), 63);
    ok_int(RtlFindLeastSignificantBit(0x8000000000000000ULL), 63);
    ok_int(RtlFindLeastSignificantBit(0xFFFFFFFFFFFFFFFFULL), 0);
    ok_int(RtlFindLeastSignificantBit(0x0000000070000000ULL), 28);
}

void
Test_RtlInitializeBitMap(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];
    BOOLEAN Exception = FALSE;

    _SEH2_TRY
    {
        RtlInitializeBitMap(NULL, NULL, 0);
    }
    _SEH2_EXCEPT(EXCEPTION_EXECUTE_HANDLER)
    {
        Exception = TRUE;
    }
    _SEH2_END;
    ok_int(Exception, 1);

    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(BitMapHeader.SizeOfBitMap, 0);
    ok_ptr(BitMapHeader.Buffer, Buffer);
    ok_hex(Buffer[0], 0xcccccccc);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(BitMapHeader.SizeOfBitMap, 8);
    ok_hex(Buffer[0], 0xcccccccc);
}

void
Test_RtlClearAllBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);
    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlClearAllBits(&BitMapHeader);
    ok_hex(Buffer[0], 0x00000000);
    ok_hex(Buffer[1], 0xcccccccc);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlClearAllBits(&BitMapHeader);
    ok_hex(Buffer[0], 0xcccccccc);
    ok_hex(Buffer[1], 0xcccccccc);
}

void
Test_RtlSetAllBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);
    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlSetAllBits(&BitMapHeader);
    ok_hex(Buffer[0], 0xffffffff);
    ok_hex(Buffer[1], 0xcccccccc);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlSetAllBits(&BitMapHeader);
    ok_hex(Buffer[0], 0xcccccccc);
    ok_hex(Buffer[1], 0xcccccccc);
}

void
Test_RtlClearBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);

    memset(Buffer, 0xff, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 0, 0);
    ok_hex(Buffer[0], 0xffffffff);
    ok_hex(Buffer[1], 0xffffffff);

    memset(Buffer, 0xff, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 0, 1);
    ok_hex(Buffer[0], 0xfffffffe);
    ok_hex(Buffer[1], 0xffffffff);

    memset(Buffer, 0xff, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 21, 1);
    ok_hex(Buffer[0], 0xffdfffff);
    ok_hex(Buffer[1], 0xffffffff);

    memset(Buffer, 0xff, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 7, 9);
    ok_hex(Buffer[0], 0xffff007f);
    ok_hex(Buffer[1], 0xffffffff);

    memset(Buffer, 0xff, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 13, 22);
    ok_hex(Buffer[0], 0x00001fff);
    ok_hex(Buffer[1], 0xfffffff8);

    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlClearBits(&BitMapHeader, 3, 6);
    RtlClearBits(&BitMapHeader, 11, 5);
    RtlClearBits(&BitMapHeader, 21, 7);
    RtlClearBits(&BitMapHeader, 37, 4);
    ok_hex(Buffer[0], 0xc00c0404);
    ok_hex(Buffer[1], 0xcccccc0c);
}

void
Test_RtlSetBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);

    memset(Buffer, 0x00, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 0, 0);
    ok_hex(Buffer[0], 0x00000000);
    ok_hex(Buffer[1], 0x00000000);

    memset(Buffer, 0x00, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 0, 1);
    ok_hex(Buffer[0], 0x00000001);
    ok_hex(Buffer[1], 0x00000000);

    memset(Buffer, 0x00, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 21, 1);
    ok_hex(Buffer[0], 0x00200000);
    ok_hex(Buffer[1], 0x00000000);

    memset(Buffer, 0x00, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 7, 9);
    ok_hex(Buffer[0], 0x0000ff80);
    ok_hex(Buffer[1], 0x00000000);

    memset(Buffer, 0x00, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 13, 22);
    ok_hex(Buffer[0], 0xffffe000);
    ok_hex(Buffer[1], 0x00000007);

    memset(Buffer, 0xcc, sizeof(Buffer));
    RtlSetBits(&BitMapHeader, 3, 6);
    RtlSetBits(&BitMapHeader, 11, 5);
    RtlSetBits(&BitMapHeader, 21, 7);
    RtlSetBits(&BitMapHeader, 37, 4);
    ok_hex(Buffer[0], 0xcfecfdfc);
    ok_hex(Buffer[1], 0xcccccdec);
}

void
Test_RtlAreBitsClear(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);
    Buffer[0] = 0x00ff00ff;
    Buffer[1] = 0xc0cfc0cf;

    ok_hex(RtlAreBitsClear(&BitMapHeader, 0, 8), FALSE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 8, 8), TRUE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 7, 8), FALSE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 8, 9), FALSE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 24, 1), FALSE);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 31);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 24, 1), TRUE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 24, 7), TRUE);
    ok_hex(RtlAreBitsClear(&BitMapHeader, 24, 8), FALSE);

}

void
Test_RtlAreBitsSet(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    RtlInitializeBitMap(&BitMapHeader, Buffer, 19);
    Buffer[0] = 0xff00ff00;
    Buffer[1] = 0x3F303F30;

    ok_hex(RtlAreBitsSet(&BitMapHeader, 0, 8), FALSE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 8, 8), TRUE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 7, 8), FALSE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 8, 9), FALSE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 24, 1), FALSE);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 31);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 24, 1), TRUE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 24, 7), TRUE);
    ok_hex(RtlAreBitsSet(&BitMapHeader, 24, 8), FALSE);

}

void
Test_RtlNumberOfSetBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0xff00ff0f;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlNumberOfSetBits(&BitMapHeader), 0);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 4);
    ok_int(RtlNumberOfSetBits(&BitMapHeader), 4);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 31);
    ok_int(RtlNumberOfSetBits(&BitMapHeader), 19);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 56);
    ok_int(RtlNumberOfSetBits(&BitMapHeader), 30);
}

void
Test_RtlNumberOfClearBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0xff00fff0;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlNumberOfClearBits(&BitMapHeader), 0);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 4);
    ok_int(RtlNumberOfClearBits(&BitMapHeader), 4);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 31);
    ok_int(RtlNumberOfClearBits(&BitMapHeader), 12);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 56);
    ok_int(RtlNumberOfClearBits(&BitMapHeader), 26);
}

void
Test_RtlFindClearBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0x060F874D;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 0), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 0), -1);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 1), -1);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 0), 1);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 1), 1);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 2), 4);

    ok_int(RtlFindClearBits(&BitMapHeader, 2, 0), 4);
    ok_int(RtlFindClearBits(&BitMapHeader, 3, 0), -1);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 32);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 21), 16);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 12), 8);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 31), 24);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 32), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 0, 39), 0);
    ok_int(RtlFindClearBits(&BitMapHeader, 4, 0), 11);
    ok_int(RtlFindClearBits(&BitMapHeader, 5, 0), 20);
    ok_int(RtlFindClearBits(&BitMapHeader, 4, 11), 11);
    ok_int(RtlFindClearBits(&BitMapHeader, 4, 12), 20);
    ok_int(RtlFindClearBits(&BitMapHeader, 2, 11), 11);
    ok_int(RtlFindClearBits(&BitMapHeader, 2, 12), 12);
    ok_int(RtlFindClearBits(&BitMapHeader, 1, 32), 1);
    ok_int(RtlFindClearBits(&BitMapHeader, 4, 32), 11);
    ok_int(RtlFindClearBits(&BitMapHeader, 5, 32), 20);

}

void
Test_RtlFindSetBits(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0xF9F078B2;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 0), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 0), -1);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 1), -1);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 0), 1);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 1), 1);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 2), 4);

    ok_int(RtlFindSetBits(&BitMapHeader, 2, 0), 4);
    ok_int(RtlFindSetBits(&BitMapHeader, 3, 0), -1);

    RtlInitializeBitMap(&BitMapHeader, Buffer, 32);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 21), 16);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 12), 8);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 31), 24);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 32), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 0, 39), 0);
    ok_int(RtlFindSetBits(&BitMapHeader, 4, 0), 11);
    ok_int(RtlFindSetBits(&BitMapHeader, 5, 0), 20);
    ok_int(RtlFindSetBits(&BitMapHeader, 4, 11), 11);
    ok_int(RtlFindSetBits(&BitMapHeader, 4, 12), 20);
    ok_int(RtlFindSetBits(&BitMapHeader, 2, 11), 11);
    ok_int(RtlFindSetBits(&BitMapHeader, 1, 32), 1);
    ok_int(RtlFindSetBits(&BitMapHeader, 4, 32), 11);
    ok_int(RtlFindSetBits(&BitMapHeader, 5, 32), 20);

}

void
Test_RtlFindClearBitsAndSet(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0x060F874D;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 0, 0), 0);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 1, 0), -1);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 1, 1), -1);
    ok_hex(Buffer[0], 0x060F874D);

    Buffer[0] = 0x060F874D;
    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 1, 0), 1);
    ok_hex(Buffer[0], 0x60f874f);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 1, 1), 4);
    ok_hex(Buffer[0], 0x60f875f);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 1, 2), 5);
    ok_hex(Buffer[0], 0x60f877f);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 2, 0), -1);
    ok_hex(Buffer[0], 0x60f877f);

    Buffer[0] = 0x060F874D;
    RtlInitializeBitMap(&BitMapHeader, Buffer, 32);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 4, 0), 11);
    ok_hex(Buffer[0], 0x60fff4d);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 5, 0), 20);
    ok_hex(Buffer[0], 0x7ffff4d);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 4, 11), 27);
    ok_hex(Buffer[0], 0x7fffff4d);

    Buffer[0] = 0x060F874D;
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 4, 12), 20);
    ok_hex(Buffer[0], 0x6ff874d);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 2, 11), 11);
    ok_hex(Buffer[0], 0x6ff9f4d);
    ok_int(RtlFindClearBitsAndSet(&BitMapHeader, 2, 12), 13);
    ok_hex(Buffer[0], 0x6ffff4d);
}

void
Test_RtlFindSetBitsAndClear(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];

    Buffer[0] = 0xF9F078B2;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 0, 0), 0);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 0, 3), 0);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 1, 0), -1);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 1, 1), -1);
    ok_hex(Buffer[0], 0xF9F078B2);

    Buffer[0] = 0xF9F078B2;
    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 1, 0), 1);
    ok_hex(Buffer[0], 0xf9f078b0);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 1, 1), 4);
    ok_hex(Buffer[0], 0xf9f078a0);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 1, 2), 5);
    ok_hex(Buffer[0], 0xf9f07880);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 2, 0), -1);
    ok_hex(Buffer[0], 0xf9f07880);

    Buffer[0] = 0xF9F078B2;
    RtlInitializeBitMap(&BitMapHeader, Buffer, 32);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 4, 0), 11);
    ok_hex(Buffer[0], 0xf9f000b2);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 5, 0), 20);
    ok_hex(Buffer[0], 0xf80000b2);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 4, 11), 27);
    ok_hex(Buffer[0], 0x800000b2);

    Buffer[0] = 0xF9F078B2;
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 4, 12), 20);
    ok_hex(Buffer[0], 0xf90078b2);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 2, 11), 11);
    ok_hex(Buffer[0], 0xf90060b2);
    ok_int(RtlFindSetBitsAndClear(&BitMapHeader, 2, 12), 13);
    ok_hex(Buffer[0], 0xf90000b2);

}

void
Test_RtlFindNextForwardRunClear(void)
{
    RTL_BITMAP BitMapHeader;
    ULONG Buffer[2];
    ULONG Index;

    Buffer[0] = 0xF9F078B2;
    Buffer[1] = 0x3F303F30;

    RtlInitializeBitMap(&BitMapHeader, Buffer, 0);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 0, &Index), 0);
    ok_int(Index, 0);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 1, &Index), 0);
    ok_int(Index, 1);

    Index = -1;
    RtlInitializeBitMap(&BitMapHeader, Buffer, 8);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 0, &Index), 1);
    ok_int(Index, 0);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 1, &Index), 2);
    ok_int(Index, 2);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 7, &Index), 0);
    ok_int(Index, 8);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 17, &Index), 0);
    ok_int(Index, 17);
    ok_int(RtlFindNextForwardRunClear(&BitMapHeader, 39, &Index), 0);
    ok_int(Index, 39);
}

void
Test_RtlFindFirstRunClear(void)
{
}

void
Test_RtlFindLastBackwardRunClear(void)
{
}

void
Test_RtlFindClearRuns(void)
{
}

void
Test_RtlFindLongestRunClear(void)
{
}


START_TEST(RtlBitmap)
{
    Test_RtlFindMostSignificantBit();
    Test_RtlFindLeastSignificantBit();
    Test_RtlInitializeBitMap();
    Test_RtlClearAllBits();
    Test_RtlSetAllBits();
    Test_RtlClearBits();
    Test_RtlSetBits();
    Test_RtlAreBitsClear();
    Test_RtlAreBitsSet();
    Test_RtlNumberOfSetBits();
    Test_RtlNumberOfClearBits();
    Test_RtlFindClearBits();
    Test_RtlFindSetBits();
    Test_RtlFindClearBitsAndSet();
    Test_RtlFindSetBitsAndClear();
    Test_RtlFindNextForwardRunClear();
    Test_RtlFindFirstRunClear();
    Test_RtlFindLastBackwardRunClear();
    Test_RtlFindClearRuns();
    Test_RtlFindLongestRunClear();
}

