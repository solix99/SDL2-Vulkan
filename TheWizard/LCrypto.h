#pragma once

#define MAX_BUFFER 9999

#include <string>
#include <time.h>
#include <random>
#include <iostream>
#include <sstream>

using namespace std;

class LCrypto
{
public:
	LCrypto();

	void encryptData(string data);
	void decryptData(string data);

	void setPrivateKey(double key);
	void setPublicKey(double key);
	void setModulus(int modulus);

	string getDataPacket();
	string getData();

	int gcd(int, int);

	int getModulus();
	double getPublicKey();

private:
	double publicKey_;
	double privateKey_;
	int dataSize_;
	double encryptArray_[MAX_BUFFER];
	double decryptArray[MAX_BUFFER];
	stringstream dataPacket_;
	string data_;
	int nModulus_;

};

