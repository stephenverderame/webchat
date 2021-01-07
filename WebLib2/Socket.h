#pragma once
#include "Connection.h"
#include "Streamable.h"
#include <functional>

class Socket : public Streamable, public Connectable {
private:
	Connection con;
protected:
	int nvi_write(const char * data, size_t len) override;
	int nvi_read(char * data, size_t amt) const override;
	int nvi_error(int errorCode) const override;
	int minAvailableBytes() const override;
	bool nvi_available() const override;
public:
	Connection& getConnection() { return con; }
	Socket(Connection && s);
	void open() override;
};

