all: cli srv
cli:
	clang++ FtpClient.cpp var.cpp -o cli

srv:
	clang++ FtpServer.cpp var.cpp -o srv
clean:
	rm cli
	rm srv

