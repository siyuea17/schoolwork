#include "Beartoy.h"

Beartoy::Beartoy()
{
	song[0] = '\0';
	dance[0] = '\0';
}

Beartoy::Beartoy(const std::string& str1, const std::string& str2)
{
	strcpy_s(song, str1.c_str());
	strcpy_s(dance, str2.c_str());
}

void Beartoy::singing()
{
	std::cout << "----------------------Ð¡ÐÜ³ª¸è----------------------" << std::endl;
	if (song[0] == '\0') {
		std::cout << "Å¶ÎØÎØ~~~~" << std::endl;
	}
	else {
		std::cout << song << std::endl;
	}
	std::cout << "---------------------Ð¡ÐÜ³ª¸è----------------------" << std::endl;
}

void Beartoy::dancing()
{
	std::cout << "----------------------Ð¡ÐÜÌøÎè----------------------" << std::endl;
	if (dance[0] == '\0') {
		std::cout << "¶å½Å½Å&&&&" << std::endl;
	}
	else {
		std::cout << dance << std::endl;
	}
	std::cout << "----------------------Ð¡ÐÜÌøÎè----------------------" << std::endl;
}
