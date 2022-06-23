#pragma once
#include <iostream>
#include <fstream>
#include <string>
class SCENES{
public:
    enum {
        MAIN_MENU,
        NEW_ACCOUNT,
        DEPOSIT_AMOUNT,
        WITHDRAW_AMOUNT,
        BALANCE_ENQUIRY,
        ALL_ACCOUNT_HOLDER_LIST,
        CLOSE_AN_ACCOUNT,
        MODIFY_AN_ACCOUNT,
        EXIT
    };
    static const char BACK;
    static unsigned int scrollRecordsBy ;
};

class MAIN_MENU {
public:
    enum {
        NEW_ACCOUNT,
        DEPOSIT_AMOUNT,
        WITHDRAW_AMOUNT,
        BALANCE_ENQUIRY,
        ALL_ACCOUNT_HOLDER_LIST,
        CLOSE_AN_ACCOUNT,
        MODIFY_AN_ACCOUNT,
        EXIT
    };
};
class NEW_ACCOUNT {
public:
    enum {
        IN_ACCOUNT_NO,
        IN_NAME,
        IN_TYPE,
        IN_DEPOSIT,
        LOG
    };
    enum {
        TYP_SAVINGS,
        TYP_CURRENT,
        TOTAL_TYPS
    };
    static char accTYP;
    static bool allowAccountCreation;
    static unsigned int nameWid;
};
class BALANCE_ENQUIRY {
public:
    enum {
        IN_ACCOUNT_NO,
        ACCOUNT_NO,
        NAME,
        TYP,
        BALANCE
    };
    static unsigned int nameWid;
};
class DEPOSIT_AMOUNT {
public:
    enum {
        IN_ACCOUNT_NO,
        DETAILS,
        AMOUNT,
        STATUS,
    };
};
class WITHDRAW_AMOUNT {
public:
    enum {
        IN_ACCOUNT_NO,
        DETAILS,
        AMOUNT,
        STATUS,
    };
};
class CLOSE_AN_ACCOUNT {
public:
    enum {
        IN_ACCOUNT_NO,
        DETAILS,
        CONFIRMATION,
        LAST_DELETED_DETAILS,
    };
    static bool _DELETE;
    static bool CONFIRM_WINDOW;
};
class MODIFY_AN_ACCOUNT {
public:
    enum {
        IN_ACCOUNT_NO,
        NEW_NAME,
        NEW_TYPE,
        NEW_DEPOSIT,
        LOG
    };
    enum {
        TYP_SAVINGS,
        TYP_CURRENT,
        TOTAL_TYPS
    };
    static char accTYP;
    static bool allowAccountModification;
    static unsigned int nameWid;
};
class account
{
    int acno;
    char name[50];
    int deposit;
    char type;
public:
    void fill_account_details(int,std::string,char,int);	//function to get data from user
    //void modify();	//function to add new data
    bool dep(int);	//function to accept amount and add to balance amount
    bool draw(int);	//function to accept amount and subtract from balance amount
    char* report() const;	//function to show data in tabular format
    int retacno() const;	//function to return account number
    std::string retname() const;	//function to show data on screen
    int retbal() const;	//function to show data on screen
    char rettyp() const;	//function to show data on screen
    //int retdeposit() const;	//function to return balance amount
    //char rettype() const;	//function to return type of account
};
bool BMS_Input_Handler(char* Scene, char Option);
void display_all();
void display_sp();
void display_sp(int);
void deposit_deposit();
void deposit_withdraw();
bool deposit_withdraw(int, int , int );
std::string getDetails(int);
void delete_account();
std::string delete_account(int);
std::string write_account(int _acno, std::string _name, char _type, int _deposit);
account* accExists(int n);
void create_account();
std::string modify_account(int n, std::string _name, char _type, size_t _deposit);
void modify_account();
void resetGlobVars();

