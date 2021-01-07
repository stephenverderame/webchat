#pragma once
#include "Streamable.h"
///Coders work sort of opposite to other streams
///Takes what you write to it (output buffer), encodes/decodes it and stores it into the readable memory (input buffer)
class Coder : public Streamable
{
protected:
	const Streamable * str;
protected:
	virtual int nvi_encode() throw(StreamException) = 0;
	virtual int nvi_decode() throw(StreamException) = 0;
public:
	/**
	 * Primes the encoder and stores the encoded data in this Streams input buffer
	 * @return the amount of encoded data in the input stream
	 * @throw StreamException on error
	 */
	int encode() throw(StreamException);
	/**
	 * Primes the decoder and stores the decoded data in this Streams input buffer
	 * @return the amount of encoded data in the input stream
	 * @throw StreamException on error
	 */
	int decode() throw(StreamException);

	/**
	 * Constructs a Coder with the specified Stream's input buffer as the Coder input
	 * @param input the Stream to encode/decode
	 */
	Coder(const Streamable * input);
	/** 
	 * Constructs a coder with the output buffer as the input for encoding/decoding
	 */
	Coder();
};
class GzipCoder : public Coder {
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amt) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;

	int nvi_encode() throw(StreamException) override;
	int nvi_decode() throw(StreamException) override;
public:
	GzipCoder(const Streamable * input) : Coder(input) {}
	GzipCoder() : Coder() {}
};

