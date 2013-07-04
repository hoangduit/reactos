#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

#define NR_THREADS (10)

ULONG nr;

DWORD WINAPI thread_main1(LPVOID param)
{
   ULONG s;

   printf("Thread %ld running\n", (DWORD)param);
   s = nr = ((nr * 1103515245) + 12345) & 0x7fffffff;
   s = s % 10;
   printf("s %ld\n", s);
   Sleep(s);
   printf("Thread %ld finished\n", (DWORD)param);
   return 0;
}

// Shows the help on how to use these program to the user
void showHelp(void)
{

printf("\nReactOS threads test program (built on %s).\n\n", __DATE__);
printf("syntax:\tthread.exe <seed>\n");
printf("\twhere <seed> is an integer number\n");
printf("\texample: thread.exe 100\n");


}

int main (int argc, char* argv[])
{
   DWORD i=0;
   DWORD id;
   ULONG nr;
   HANDLE ThreadHandle[NR_THREADS];

   // The user must supply one argument (the seed). if he/she doesn't
   // then we show the help.
   //   if(argc < 2) {
   //		showHelp();
   //		return 1;
   //   }

   //   nr = atoi(argv[1]);
   nr = 500;
   printf("Seed %ld\n", nr);

   printf("Creating %d threads...\n",NR_THREADS*2);
   for (i=0;i<NR_THREADS;i++)
     {
       ThreadHandle[i] = CreateThread(NULL,
				      0,
				      thread_main1,
				      (LPVOID)i,
				      CREATE_SUSPENDED,
				      &id);

     }

   for (i=0;i<NR_THREADS;i++)
     {
       ResumeThread(ThreadHandle[i]);
     }

   for (i=0;i<NR_THREADS;i++)
     {
       SuspendThread(ThreadHandle[i]);
     }

   for (i=0;i<NR_THREADS;i++)
     {
       ResumeThread(ThreadHandle[i]);
     }

   printf("All threads created...\n");
   return 0;
}
