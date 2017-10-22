#pragma once


//#include <unistd.h>

#include <string>

#include "google/protobuf/message.h"

using ::google::protobuf::Message;

bool ReadProtoFromTextFile(const char* filename, Message* proto);

inline bool ReadProtoFromTextFile(const std::string& filename, Message* proto) {
	return ReadProtoFromTextFile(filename.c_str(), proto);
}

inline void ReadProtoFromTextFileOrDie(const char* filename, Message* proto) {
	assert(ReadProtoFromTextFile(filename, proto));
}

inline void ReadProtoFromTextFileOrDie(const std::string& filename, Message* proto) {
	ReadProtoFromTextFileOrDie(filename.c_str(), proto);
}

bool WriteProtoToTextFile(const Message& proto, const char* filename);
inline bool WriteProtoToTextFile(const Message& proto, const std::string& filename) {
	return WriteProtoToTextFile(proto, filename.c_str());
}

bool ReadProtoFromBinaryFile(const char* filename, Message* proto);

inline bool ReadProtoFromBinaryFile(const std::string& filename, Message* proto) {
	return ReadProtoFromBinaryFile(filename.c_str(), proto);
}

inline void ReadProtoFromBinaryFileOrDie(const char* filename, Message* proto) {
	assert(ReadProtoFromBinaryFile(filename, proto));
}

inline void ReadProtoFromBinaryFileOrDie(const std::string& filename, Message* proto) {
	ReadProtoFromBinaryFileOrDie(filename.c_str(), proto);
}


bool WriteProtoToBinaryFile(const Message& proto, const char* filename);
inline bool WriteProtoToBinaryFile(const Message& proto, const std::string& filename) {
	return WriteProtoToBinaryFile(proto, filename.c_str());
}
