#include "BMS_commonDef.h"

//#include <cctype>
//#include <iomanip>
unsigned int SCENES::scrollRecordsBy = 0; // defining scrollRecords (since its static member)
unsigned int BALANCE_ENQUIRY::nameWid = 400; // defining nameWid (since its static member)
bool CLOSE_AN_ACCOUNT::_DELETE = false;
bool CLOSE_AN_ACCOUNT::CONFIRM_WINDOW = false;
char NEW_ACCOUNT::accTYP = 0;
bool NEW_ACCOUNT::allowAccountCreation = false;
size_t NEW_ACCOUNT::nameWid = 400;
char MODIFY_AN_ACCOUNT::accTYP = 0;
bool MODIFY_AN_ACCOUNT::allowAccountModification = false;
size_t MODIFY_AN_ACCOUNT::nameWid = 400;
const char SCENES::BACK = -1;

bool BMS_Input_Handler(char* Scene, char Option) {
	if (Option == SCENES::BACK) {
		(*Scene) = SCENES::MAIN_MENU;
		return false;
	}
	switch (*Scene) {
	case SCENES::MAIN_MENU:
		resetGlobVars();
		switch (Option) {
		case MAIN_MENU::NEW_ACCOUNT:
			NEW_ACCOUNT::allowAccountCreation = false;
			(*Scene) = SCENES::NEW_ACCOUNT;
			break;
		case MAIN_MENU::DEPOSIT_AMOUNT:
			(*Scene) = SCENES::DEPOSIT_AMOUNT;
			break;
		case MAIN_MENU::WITHDRAW_AMOUNT:
			(*Scene) = SCENES::WITHDRAW_AMOUNT;
			break;
		case MAIN_MENU::BALANCE_ENQUIRY:
			(*Scene) = SCENES::BALANCE_ENQUIRY;
			break;
		case MAIN_MENU::ALL_ACCOUNT_HOLDER_LIST:
			(*Scene) = SCENES::ALL_ACCOUNT_HOLDER_LIST;
			SCENES::scrollRecordsBy = 0;
			break;
		case MAIN_MENU::CLOSE_AN_ACCOUNT:
			(*Scene) = SCENES::CLOSE_AN_ACCOUNT;
			CLOSE_AN_ACCOUNT::_DELETE = false;
			break;
		case MAIN_MENU::MODIFY_AN_ACCOUNT:
			(*Scene) = SCENES::MODIFY_AN_ACCOUNT;
			break;
		case MAIN_MENU::EXIT:
			(*Scene) = SCENES::EXIT;
			break;
		}
		break;
	case SCENES::NEW_ACCOUNT:
		switch (Option) {
		case NEW_ACCOUNT::IN_ACCOUNT_NO:
			return true;
		}
		if (NEW_ACCOUNT::allowAccountCreation) {
			NEW_ACCOUNT::nameWid = 400;
			switch (Option) {
			case NEW_ACCOUNT::IN_NAME:
				return true;
			case NEW_ACCOUNT::IN_TYPE:
				NEW_ACCOUNT::accTYP = (NEW_ACCOUNT::accTYP + 1) % NEW_ACCOUNT::TOTAL_TYPS;
				return false;
			case NEW_ACCOUNT::IN_DEPOSIT:
				return true;
			}
		}
		break;
	case SCENES::BALANCE_ENQUIRY:
		switch (Option) {
		case BALANCE_ENQUIRY::IN_ACCOUNT_NO:
			return true;
		}
		BALANCE_ENQUIRY::nameWid = 400;
		break;
	case SCENES::DEPOSIT_AMOUNT:
		switch (Option) {
		case DEPOSIT_AMOUNT::IN_ACCOUNT_NO:
			return true;
		case DEPOSIT_AMOUNT::AMOUNT:
			return true;
		}
		break;
	case SCENES::WITHDRAW_AMOUNT:
		switch (Option) {
		case WITHDRAW_AMOUNT::IN_ACCOUNT_NO:
			return true;
		case WITHDRAW_AMOUNT::AMOUNT:
			return true;
		}
		break;
	case SCENES::CLOSE_AN_ACCOUNT:
		switch (Option) {
		case CLOSE_AN_ACCOUNT::IN_ACCOUNT_NO:
			return true;
		case CLOSE_AN_ACCOUNT::CONFIRMATION:
			return true;
		}
		break;
	case SCENES::MODIFY_AN_ACCOUNT:
		switch (Option) {
		case MODIFY_AN_ACCOUNT::IN_ACCOUNT_NO:
			return true;
		}
		if (MODIFY_AN_ACCOUNT::allowAccountModification) {
			MODIFY_AN_ACCOUNT::nameWid = 400;
			switch (Option) {
			case MODIFY_AN_ACCOUNT::NEW_NAME:
				return true;
			case MODIFY_AN_ACCOUNT::NEW_TYPE:
				MODIFY_AN_ACCOUNT::accTYP = (MODIFY_AN_ACCOUNT::accTYP + 1) % MODIFY_AN_ACCOUNT::TOTAL_TYPS;
				return false;
			case MODIFY_AN_ACCOUNT::NEW_DEPOSIT:
				return true;
			}
		}
		break;
	}
	return false;
}
char* account::report() const
{
	int len = strlen(name);
	char newName[21] = { "\0" };
	if (len < 20) {
		for (size_t i = 0;i < len;i++)
			newName[i] = name[i];
		for (size_t i = len;i < 20;i++)
			newName[i] = 32;
		newName[20] = '\0';
		//cout << acno << setw(10) << " " << elongatedName << setw(10) << " " << type << setw(6) << deposit << endl;
	}
	else if (len > 20) {
		for (int i = 0;i < 17;i++)
			newName[i] = name[i];
		for (int i = 17;i < 20;i++)
			newName[i] = '.';
		newName[20] = '\0';
		//cout << acno << setw(10) << " " << shortenedName << "..." << setw(10) << " " << type << setw(6) << deposit << endl;
	}
	else {
		for (int i = 0;i < 21;i++)
			newName[i] = name[i];
	}
	//std::cout << len << ","<<strlen(newName)<< std::endl;
	std::string AcNo = std::to_string(acno);
	len = AcNo.size();
	char newAcNo[8] = { '\0' };
	if (len < 7) {
		for (size_t i = 0;i < 7-len;i++)
			newAcNo[i] = 32;
		for (size_t i = 7 - len,j = 0;i < 8;i++,j++)
			newAcNo[i] = AcNo[j];
		//cout << acno << setw(10) << " " << elongatedName << setw(10) << " " << type << setw(6) << deposit << endl;
	}
	else {
		for (int i = 0;i < 8;i++)
			newAcNo[i] = AcNo[i];
	}
	char* locBuff = new char[100]{"\0"};
	snprintf(locBuff, sizeof(char)*100, "%s       %s           %c       %d", newAcNo,newName,type,deposit);
	return locBuff;
}
int account::retacno() const
{
	return acno;
}
std::string account::retname() const
{
	return std::string(name);
}
int account::retbal() const
{
	return deposit;
}
char account::rettyp() const
{
	return type;
}
void account::fill_account_details(int _acno, std::string _name, char _type, int _deposit)
{
	acno = _acno;
	for(size_t i=0;i<_name.size()+1;i++)
		name[i] = _name[i];
	type = _type;
	deposit = _deposit;
}
bool account::dep(int x)
{
	deposit += x;
	return true;
}
bool account::draw(int x)
{
	int bal = deposit - x;
	if ((bal < 500 && type == 'S') || (bal < 1000 && type == 'C'))
		return false;
	else
		deposit -= x;
	return true;
}
std::string delete_account(int n)
{
	account ac;
	std::ifstream inFile;
	std::ofstream outFile;
	std::string deletedDetails;
	inFile.open("account.dat", std::ios::binary);
	if (!inFile)
	{
		std::cout << "File could not be open !! Press any Key...";
		return "-";
	}
	outFile.open("Temp.dat", std::ios::binary);
	inFile.seekg(0, std::ios::beg);
	while (inFile.read(reinterpret_cast<char*> (&ac), sizeof(account)))
	{
		if (ac.retacno() != n)
		{
			outFile.write(reinterpret_cast<char*> (&ac), sizeof(account));
		}
		else {
			deletedDetails = ac.report();
		}
	}
	inFile.close();
	outFile.close();
	remove("account.dat");
	rename("Temp.dat", "account.dat");
	return deletedDetails;
}
std::string write_account(int _acno, std::string _name, char _type, int _deposit)
{
	account ac;
	std::string errlog = "";
	std::ofstream outFile;
	if (_name == "" || _type == 0 || _deposit < 1)
		errlog = "Fill The Subsequent Feilds";
	outFile.open("account.dat", std::ios::binary | std::ios::app);
	if (errlog == "") {
		if ((_deposit < 500 && _type == 'S') || (_deposit < 1000 && _type == 'C'))
			errlog = "Insufficient Deposit For New ACCOUNT";
		if (!(_type == 'S' || _type == 'C'))
			errlog = "Ivalid Account Type.";
		if (accExists(_acno))
			errlog = "Account No. Already Exists.";
	}
	if (errlog == "") {
		ac.fill_account_details(_acno, _name, _type, _deposit);
		outFile.write(reinterpret_cast<char*> (&ac), sizeof(account));
	}
	outFile.close();
	return errlog;
}
account* accExists(int n) {
	account* ac = new account();
	bool flag = false;
	std::ifstream inFile;
	inFile.open("account.dat", std::ios::binary);
	if (!inFile)
	{
		std::cout << "File could not be open !! Press any Key...";
		return nullptr;
	}
	while (inFile.read(reinterpret_cast<char*> (ac), sizeof(account)))
	{
		if (ac->retacno() == n)
			return ac;
	}
	return nullptr;
}
std::string modify_account(int n,std::string _name,char _type,size_t _deposit)
{
	bool found = false;
	std::string errlog;
	account ac;
	std::fstream File;
	
	if (_name == "" || _type == 0 || _deposit < 1)
		errlog = "Fill The Subsequent Feilds";
	
	if (errlog == "") {
		if ((_deposit < 500 && _type == 'S') || (_deposit < 1000 && _type == 'C'))
			errlog = "Insufficient Deposit For New ACCOUNT";
		if (!(_type == 'S' || _type == 'C'))
			errlog = "Ivalid Account Type.";
		if (!accExists(n))
			errlog = "Account No. Not Exists.";
	}
	File.open("account.dat", std::ios::binary | std::ios::in | std::ios::out);
	if (!File)
	{
		std::cout << "File could not be open !";
		return "File could not be open !";
	}
	if (errlog == "") {
		while (!File.eof() && found == false)
		{
			File.read(reinterpret_cast<char*> (&ac), sizeof(account));
			if (ac.retacno() == n)
			{
				ac.fill_account_details(ac.retacno(), _name, _type, _deposit);
				int pos = (-1) * static_cast<int>(sizeof(account));
				File.seekp(pos, std::ios::cur);
				File.write(reinterpret_cast<char*> (&ac), sizeof(account));
				std::cout << "\n\n\t Record Updated";
				found = true;
			}
		}
	}
	File.close();
	return errlog;
}