#pragma once
#include "Streamable.h"
//Coders work sort of opposite to other streams
//Takes what you write to it (output buffer), encodes/decodes it and stores it into the readable memory (input buffer)
class Coder : public Streamable
{
protected:
	const Streamable * str;
protected:
	virtual int nvi_encode() = 0;
	virtual int nvi_decode() = 0;
public:
	//Primes the encoder/decoder. Results are stored in input buffer of the current stream
	//Reads from the bound Streamable (which must be already synced) or the output buffer
	//Always copies data
	int encode();
	int decode();

	Coder(const Streamable * input);
	Coder();
};
class GzipCoder : public Coder {
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amt) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;

	int nvi_encode() override;
	int nvi_decode() override;
public:
	GzipCoder(const Streamable * input) : Coder(input) {}
	GzipCoder() : Coder() {}
};

