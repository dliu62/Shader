#include "stdafx.h"
#include "io.h"

#include <fcntl.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>
#include <google/protobuf/text_format.h>

#include <stdint.h>

#include <fstream>  // NOLINT(readability/streams)
#include <string>
#include <vector>
#include <io.h> //for windows
//#include <unistd.h> //for unix
//#include <WinSock2.h>
//#include <WS2tcpip.h>


const int kProtoReadBytesLimit = INT_MAX;  // Max size of 2 GB minus 1 byte.

using google::protobuf::io::FileInputStream;
using google::protobuf::io::FileOutputStream;
using google::protobuf::io::ZeroCopyInputStream;
using google::protobuf::io::CodedInputStream;
using google::protobuf::io::ZeroCopyOutputStream;
using google::protobuf::io::CodedOutputStream;
using google::protobuf::Message;

bool ReadProtoFromTextFile(const char* filename, Message* proto) {
	//int fd = _open(filename, O_RDONLY);
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		printf("ReadProtoFromTextFile File not found!");
		return false;
	}
	FileInputStream* input = new FileInputStream(fd);
	bool success = google::protobuf::TextFormat::Parse(input, proto);
	delete input;
	close(fd);
	return success;
}

bool WriteProtoToTextFile(const Message& proto, const char* filename) {
	int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (fd == -1)
	{
		printf("WriteProtoToTextFile File not found!");
		return false;
	}
	FileOutputStream* output = new FileOutputStream(fd);
	bool success = google::protobuf::TextFormat::Print(proto, output);
	delete output;
	close(fd);
	return success;
}

bool ReadProtoFromBinaryFile(const char* filename, Message* proto) {
	int fd = open(filename, O_RDONLY);
	if (fd == -1)
	{
		printf("ReadProtoFromBinary File File not found!");
		return false;
	}
	ZeroCopyInputStream* raw_input = new FileInputStream(fd);
	CodedInputStream* coded_input = new CodedInputStream(raw_input);
	coded_input->SetTotalBytesLimit(kProtoReadBytesLimit, 536870912);

	bool success = proto->ParseFromCodedStream(coded_input);

	delete coded_input;
	delete raw_input;
	close(fd);
	return success;
}

bool WriteProtoToBinaryFile(const Message& proto, const char* filename) {
	std::fstream output(filename, std::ios::out | std::ios::trunc | std::ios::binary);
	return proto.SerializeToOstream(&output);
}


