#pragma once

typedef struct _ACCELERATOR_TABLE
{
  HEAD head;
  ULONG Count;
  LPACCEL Table;
} ACCELERATOR_TABLE, *PACCELERATOR_TABLE;

PACCELERATOR_TABLE FASTCALL UserGetAccelObject(HACCEL);
