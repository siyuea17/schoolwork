#include "Cattoy.h"

Cattoy::Cattoy()
{
	song[0] = '\0';
	dance[0] = '\0';
}

Cattoy::Cattoy(const std::string& str1, const std::string& str2)
{
	strcpy_s(song, str1.c_str());
	strcpy_s(dance, str2.c_str());
}

void Cattoy::singing()
{
	std::cout << "----------------------Ã¨Ã¨³ª¸è----------------------" << std::endl;
	if (song[0] == '\0') {
		std::cout << "ß÷ß÷ß÷~~~~" << std::endl;
	}
	else {
		std::cout << song << std::endl;
	}
	std::cout << "----------------------Ã¨Ã¨³ª¸è----------------------" << std::endl;
}

void Cattoy::dancing()
{
	std::cout << "----------------------Ã¨Ã¨ÌøÎè----------------------" << std::endl;
	if (dance[0] == '\0') {
		std::cout << "×ªÈ¦È¦@@@" << std::endl;
	}
	else {
		std::cout << dance << std::endl;
	}
	std::cout << "----------------------Ã¨Ã¨ÌøÎè----------------------" << std::endl;
}