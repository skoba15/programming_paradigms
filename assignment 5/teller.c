#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <inttypes.h>

#include "teller.h"
#include "account.h"
#include "error.h"
#include "debug.h"
#include "branch.h"

/*
 * deposit money into an account
 */
int
Teller_DoDeposit(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);

  DPRINTF('t', ("Teller_DoDeposit(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
 
  pthread_mutex_lock(&(account->accountlocker));
  BranchID branchid=AccountNum_GetBranchID(accountNum);
  
  pthread_mutex_lock(&(bank->branches[branchid].branchlockerr));
  Account_Adjust(bank,account, amount, 1);
  pthread_mutex_unlock(&(account->accountlocker));
  pthread_mutex_unlock(&(bank->branches[branchid].branchlockerr));
  return ERROR_SUCCESS;
}

/*
 * withdraw money from an account
 */
int
Teller_DoWithdraw(Bank *bank, AccountNumber accountNum, AccountAmount amount)
{
  assert(amount >= 0);
  DPRINTF('t', ("Teller_DoWithdraw(account 0x%"PRIx64" amount %"PRId64")\n",
                accountNum, amount));

  Account *account = Account_LookupByNumber(bank, accountNum);
  
  if (account == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  pthread_mutex_lock(&(account->accountlocker));
  BranchID branchid=AccountNum_GetBranchID(accountNum);
  
  pthread_mutex_lock(&(bank->branches[branchid].branchlockerr));

  
  if (amount > Account_Balance(account)) {
  pthread_mutex_unlock(&(account->accountlocker));
  pthread_mutex_unlock(&(bank->branches[branchid].branchlockerr));
    return ERROR_INSUFFICIENT_FUNDS;
  }
  
  Account_Adjust(bank,account, -amount, 1);
  pthread_mutex_unlock(&(account->accountlocker));
  pthread_mutex_unlock(&(bank->branches[branchid].branchlockerr));
  return ERROR_SUCCESS;
}

/*
 * do a tranfer from one account to another account
 */
int
Teller_DoTransfer(Bank *bank, AccountNumber srcAccountNum,
                  AccountNumber dstAccountNum,
                  AccountAmount amount)
{
  if(srcAccountNum==dstAccountNum)return 0;
  assert(amount >= 0);
  
  DPRINTF('t', ("Teller_DoTransfer(src 0x%"PRIx64", dst 0x%"PRIx64
                ", amount %"PRId64")\n",
                srcAccountNum, dstAccountNum, amount));
  
  Account *srcAccount = Account_LookupByNumber(bank, srcAccountNum);
  if (srcAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }

  Account *dstAccount = Account_LookupByNumber(bank, dstAccountNum);
  if (dstAccount == NULL) {
    return ERROR_ACCOUNT_NOT_FOUND;
  }
	int notsame=Account_IsSameBranch(srcAccountNum, dstAccountNum);
	
 
     if(notsame){
	if(srcAccount->priority<dstAccount->priority){
           
		pthread_mutex_lock(&(srcAccount->accountlocker));
           
                pthread_mutex_lock(&(dstAccount->accountlocker));
        }
	else{
		
                pthread_mutex_lock(&(dstAccount->accountlocker));
   
		pthread_mutex_lock(&(srcAccount->accountlocker));
	}




	if (amount > Account_Balance(srcAccount)) {
                           pthread_mutex_unlock(&(srcAccount->accountlocker));
                pthread_mutex_unlock(&(dstAccount->accountlocker));
                   
    			return ERROR_INSUFFICIENT_FUNDS;
  		}
                 else{
			 Account_Adjust(bank, srcAccount, -amount, 0);
  Account_Adjust(bank, dstAccount, amount, 0);
		       pthread_mutex_unlock(&(srcAccount->accountlocker));
                pthread_mutex_unlock(&(dstAccount->accountlocker));
              
			return ERROR_SUCCESS;
                  }
   }
   else{
	 BranchID branchid1=AccountNum_GetBranchID(srcAccountNum);
	 BranchID branchid2=AccountNum_GetBranchID(dstAccountNum);
	if(branchid1<branchid2){
	
		pthread_mutex_lock(&(srcAccount->accountlocker));
            
		pthread_mutex_lock(&(dstAccount->accountlocker));
                
		pthread_mutex_lock(&(bank->branches[branchid1].branchlockerr));
                
		pthread_mutex_lock(&(bank->branches[branchid2].branchlockerr));
	}
	else{
              
		pthread_mutex_lock(&(dstAccount->accountlocker));
		pthread_mutex_lock(&(srcAccount->accountlocker));
		pthread_mutex_lock(&(bank->branches[branchid2].branchlockerr));
		pthread_mutex_lock(&(bank->branches[branchid1].branchlockerr));
	}

	if (amount > Account_Balance(srcAccount)) {
                  	pthread_mutex_unlock(&(srcAccount->accountlocker));
		pthread_mutex_unlock(&(dstAccount->accountlocker));
		pthread_mutex_unlock(&(bank->branches[branchid1].branchlockerr));
		pthread_mutex_unlock(&(bank->branches[branchid2].branchlockerr));
                      
    			return ERROR_INSUFFICIENT_FUNDS;
  		}
        else{

		pthread_mutex_lock(&(bank->bankbalancelocker));
			 Account_Adjust(bank, srcAccount, -amount, 1);
  Account_Adjust(bank, dstAccount, amount, 1);
		      pthread_mutex_unlock(&(srcAccount->accountlocker));
		pthread_mutex_unlock(&(dstAccount->accountlocker));
		pthread_mutex_unlock(&(bank->branches[branchid1].branchlockerr));
		pthread_mutex_unlock(&(bank->branches[branchid2].branchlockerr));	
		pthread_mutex_unlock(&(bank->bankbalancelocker));
		
                   
return ERROR_SUCCESS;
          }
  }



  /*
   * If we are doing a transfer within the branch, we tell the Account module to
   * not bother updating the branch balance since the net change for the
   * branch is 0.
   */

}
