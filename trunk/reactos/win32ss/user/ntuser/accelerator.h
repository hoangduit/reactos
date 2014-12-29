#pragma once

typedef struct _ACCELERATOR_TABLE
{
  PROCMARKHEAD head;
  ULONG Count;
  LPACCEL Table;
} ACCELERATOR_TABLE, *PACCELERATOR_TABLE;

PACCELERATOR_TABLE FASTCALL UserGetAccelObject(HACCEL);
BOOLEAN
UserDestroyAccelTable(PVOID Object);
