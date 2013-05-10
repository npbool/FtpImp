all: cli srv
cli:
	g++ FtpClient.cpp var.cpp -o cli

srv:
	g++ FtpServer.cpp var.cpp -o srv
clean:
	rm cli
	rm srv

