#ifndef _BANK_H
#define _BANK_H

#include <pthread.h>
#include <semaphore.h>


typedef struct Bank {
  unsigned int numberBranches;
  struct       Branch  *branches;
  struct       Report  *report;
  pthread_mutex_t transferlocker;
  int numofworkers;
  int endedworkingday;
  pthread_mutex_t countchecker;
  sem_t semm;
  pthread_mutex_t bankbalancelocker;
} Bank;

#include "account.h"

int Bank_Balance(Bank *bank, AccountAmount *balance);

Bank *Bank_Init(int numBranches, int numAccounts, AccountAmount initAmount,
                AccountAmount reportingAmount,
                int numWorkers);

int Bank_Validate(Bank *bank);
int Bank_Compare(Bank *bank1, Bank *bank2);
void disposebank(Bank * bank);


#endif /* _BANK_H */
