#include "LCrypto.h"

LCrypto::LCrypto()
{
 
    
}

void LCrypto::encryptData(string data)
{
    dataSize_ = data.length();

    for (unsigned int i = 0; i < dataSize_; i++)
    {
        encryptArray_[i] = pow(data.at(i), publicKey_);
        //encryptArray_[i] = fmod(encryptArray_[i], nModulus_);
    }

    dataPacket_.clear();
    dataPacket_.str(string());

    for (unsigned int i = 0; i < dataSize_; i++)
    {
        dataPacket_ << encryptArray_[i] << ",";
    }
}

void LCrypto::decryptData(string data)
{
    dataSize_ = data.length();

    data_.resize(dataSize_, ' ');

    for (unsigned int i = 0; i < dataSize_; i++)
    {
        decryptArray[i] = pow(decryptArray[i], privateKey_);
        decryptArray[i] = fmod(decryptArray[i], nModulus_);
        data_.at(i) = char(decryptArray[i] + 1);
    }
}

void LCrypto::setPrivateKey(double key)
{
    privateKey_ = key;
}
void LCrypto::setPublicKey(double key)
{
    publicKey_ = key;
}
string LCrypto::getData()
{
    return data_;
}
string LCrypto::getDataPacket()
{
    return dataPacket_.str();
}

int LCrypto::gcd(int a, int b)
{
    int t;
    while (1)
    {
        t = a % b;
        if (t == 0)
            return b;
        a = b;
        b = t;
    }
}
void LCrypto::setModulus(int modulus)
{
    nModulus_ = modulus;
}
int LCrypto::getModulus()
{
    return nModulus_;
}
double LCrypto::getPublicKey()
{
    return publicKey_;
}