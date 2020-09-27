inb4-4plebs: *.cpp *.h
	c++ -o inb4-4plebs *.cpp -I/usr/local/include -I/usr/local/include/mongocxx/v_noabi -I/usr/local/include/bsoncxx/v_noabi -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lcurl -lmongocxx -lbsoncxx

install: inb4-4plebs
	install inb4-4plebs /bin/
