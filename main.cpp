//Dan Krupsky, 108

/*
Всё компилим через MinGW
http://sourceforge.net/projects/boost/files/boost/1.57.0/ - тут скачать (zip файл)
!!! Как установить BOOST:
записать в PATH путь к MinGW\bin
Зайти в директории буста tools\build\v2
выполнить
bootstrap mingw
tools\build\v2\b2 toolset=gcc --build-type=complete stage --with-filesystem
Библиотеки будут в директория буста  \ stage\lib
Переименовать соответствующие .a файлы (нам нужны filesystem и system) соответсвенно в libboost_filesystem.a и libboost_system.a
!!! Как установить протобаф:
распаковать протобаф в home директории msys'a в mingw, зайти в эту директорию
./configure --prefix=`cd C:/MinGW; pwd -W`
make
make install

зайти в папку с исходниками, теперь proto файл можно компилировать командой protoc:
protoc example.proto
На выходу получается два файла: example.pb.h и example.pb.cc, в main.cpp делаем инклуд на .h файл

!!! Команда для компиляции (из MSYS):
g++ main.cpp pbfile.pb.cc -std=c++11 -IC:/MinGW/msys/1.0/home/boost_1_57_0/ -LC:/MinGW/msys/1.0/home/boost_1_57_0/stage/lib -lboost_filesystem -lboost_system -lprotobuf

Для работы не из msys'a надо найти следующие dll в директррии mingw и сложить их в папку с exe файлом:
libgcc_s_dw2-1.dll
libprotobuf-9.dll
libstdc++-6.dll
*/

#include <iostream>
#include <fstream>  //Чтение и запись
#include <string>
#include <boost/filesystem.hpp>  //BOOST::FILESYSTEM с помощью которого считываем директорию
#include "pbfile.pb.h" //файл, который сделал protoc
#include "md6/md6.h"
#include "tinyxml/tinyxml.h"
#include "tinyxml/tinyxmlerror.cpp"
#include "tinyxml/tinyxmlparser.cpp"

namespace fs = boost::filesystem;
std::string test;

namespace sha1
{
	void calc(const void* src, const int bytelength, unsigned char* hash);

	void toHexString(const unsigned char* hash, char* hexstring);

} // namespace sha1

std::string hash_sha1(const char * hash_str, int length) {
	unsigned char * hash = new unsigned char[length];
	char * hexstring = new char[41]; // Строка хэш-суммы всегда одной длины
	sha1::calc(hash_str, length, hash);
	sha1::toHexString(hash, hexstring);

	return hexstring;
	delete[] hash;
	delete[] hexstring;
}

//Данные будем записывать в структуру, а структуру копировать в вектор
struct Fileinfo {
	std::string path;
	std::string hash;
	std::string hash_sha_1;
	int size;
	std::string flag = "NEW";
};

void save2xml(string filename, vector<Fileinfo> vec_finfo) {
	TiXmlDocument doc;
	TiXmlDeclaration * decl = new TiXmlDeclaration("1.0", "", "");
	doc.LinkEndChild(decl);
	for (Fileinfo it : vec_finfo) {
		TiXmlElement * element = new TiXmlElement("File");
		doc.LinkEndChild(element);
		element->SetAttribute("path", it.path.c_str());
		element->SetAttribute("size", it.size);
		element->SetAttribute("hash", it.hash.c_str());
		//element->SetAttribute("flag", it.flag);
		TiXmlText * text = new TiXmlText("");
		element->LinkEndChild(text);
	}
	doc.SaveFile(filename.c_str());
}

void loadxml(string filename, vector<Fileinfo> & vec_finfo) {
	Fileinfo it;
	TiXmlDocument doc;
	doc.LoadFile(filename.c_str());
	TiXmlHandle docHandle(&doc);
	TiXmlElement* child = docHandle.FirstChild("File").ToElement();
	for (child; child; child = child->NextSiblingElement())
	{

		it.size = atoi(child->Attribute("size"));
		it.hash = child->Attribute("hash");
		it.path = child->Attribute("path");
		vec_finfo.push_back(it);
	}
}

//Сравниваем старый и новый список файлов
std::vector<Fileinfo> compare_lists(std::vector<Fileinfo> newfl, std::vector<Fileinfo> oldfl) {
	for (std::vector<Fileinfo>::iterator itnew = newfl.begin(); itnew < newfl.end(); itnew++) {
		/*Бегаем по вектору:
		//Если нашли путь и хеш совпал, то меняем флаг на UNCHANGED
		//Если нашли путь и хеш не совпал, то меняем флаг на CHANGED
		//В новом файле у всех флагов у всех структур стоит флаг NEW - если не нашли в старом, то его и оставляем
		//Всё что осталось в старом - DELETED, добавляем в newfl и возвращаем его же
		*/
		for (std::vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
			if ((itnew->path == itold->path) && (itnew->hash == itold->hash)) {
				itnew->flag = "UNCHANGED";
				oldfl.erase(itold);
				break;
			}
			if ((itnew->path == itold->path) && (itnew->hash != itold->hash)) {
				itnew->flag = "CHANGED";
				oldfl.erase(itold);
				break;
			}
		}
	}
	for (std::vector<Fileinfo>::iterator itold = oldfl.begin(); itold < oldfl.end(); itold++) {
		itold->flag = "DELETED";
		newfl.push_back(*itold);
	}
	return newfl;
}

//Записываем через протобаф. Filelist - внешняя структура, в которую кладем элементы Filep
void savepbuf(std::string filename, std::vector<Fileinfo> & vec_finfo) {
	nsofdir::ArrFilep flist;
	nsofdir::Filep * file_entry;
	std::ofstream output(filename, std::ofstream::binary);
	for (Fileinfo it : vec_finfo) {
		//Запись, просто по сделанным методам протобафа
		file_entry = flist.add_filep();
		file_entry->set_filepath(it.path);
		file_entry->set_size(it.size);
		file_entry->set_mdsixhash(it.hash);
		file_entry->set_sha1hash(it.hash_sha_1);

	}
	//Вывод файла
	flist.PrintDebugString();
	//Записываем в output файл
	flist.SerializeToOstream(&output);
	output.close();
}

//Загрузка записанного файла (записывает в файл, но пока не засовывает в вектор - доделать позже
void loadpbuf(std::string filename, std::vector<Fileinfo> & vec_finfo) {
	//вспомогательная структура Fileinfo, через которую заполним вектор
	Fileinfo it;
	//Filelist, в который считаем файл
	nsofdir::ArrFilep flist;  
	nsofdir::Filep file_entry;
	// Открываем наш записанный файл
	std::ifstream input(filename, std::ofstream::binary); 
	//Парсим из файла
	flist.ParseFromIstream(&input);  
	input.close();
	//flist.PrintDebugString(); // Вывод файла
	file_entry.PrintDebugString();
	for (int i = 0; i < flist.filep_size(); i++) {
		file_entry = flist.filep(i);
		it.path = file_entry.filepath();
		it.size = file_entry.size();
		it.hash = file_entry.mdsixhash();
		vec_finfo.push_back(it);
	}
}

//выводит список файлов и папок в директории
void get_dir_list(fs::directory_iterator iterator, std::vector<Fileinfo> & vec_finfo, Fileinfo & finfo, std::ifstream & ifs) {
	for (; iterator != fs::directory_iterator(); ++iterator)
	{
		if (fs::is_directory(iterator->status())) {
			//если наткнулись на папку, то рекурсивно запускаем эту же функцию для этой папки
			fs::directory_iterator sub_dir(iterator->path());
			get_dir_list(sub_dir, vec_finfo, finfo, ifs);

		}
		else
			//а если нет, то записываем в структуру имя, размер, хеш, и флажок (понадобится чуть позже, когда будем искать изменения в файлах)
		{

			finfo.path = iterator->path().string();
			//исправляем косяк с \ перед файлом в пути, так работает filesystem :-(
			std::replace(finfo.path.begin(), finfo.path.end(), '\\', '/');
			finfo.size = fs::file_size(iterator->path());
			ifs.open(finfo.path, std::ios_base::binary);
			std::string strifs((std::istreambuf_iterator<char>(ifs)),
				(std::istreambuf_iterator<char>()));
			finfo.hash = md6(strifs);
			finfo.hash_sha_1 = hash_sha1(strifs.c_str(), strifs.length());
			ifs.close();
			vec_finfo.push_back(finfo);
		}

	}
}

void print_finfo_vec(std::vector<Fileinfo> vec) {
	for (Fileinfo element : vec) {
		std::cout << element.path << std::endl <<
			element.size << std::endl <<
			element.hash << std::endl <<
			element.hash_sha_1 << std::endl <<
			element.flag << std::endl << "-------" << std::endl;
	}
}

int main() {
	std::ofstream myfile;
	std::string path, dirpath;
	//объявление структуры, в которую будем записывать данные и складывать их в вектор
	Fileinfo finfo;
	//Инпут, через который мы считаем хеш
	std::ifstream ifs;
	std::string checkstatus;
	std::cout << "Do you wish to save filelist or check current folder with previous result?" << 
		std::endl << "(check/save or anything else for neither)" 
		<< "checkxml/savexml for xml option" << std::endl;
	std::cin >> checkstatus;
	std::cin.clear();
	fflush(stdin);
	
	std::cout << "Folder path:" << std::endl;
	std::getline(std::cin, path);
	// Кстати, путь можно скопировать и вставлять через меню, кликая по иконке запускаемого приложения в левом верхнем углу.
	// Клик левой кнопкой по иконке - изменить - вставить.
	//Вектор, в который мы будет складывать объекты нашей структуры
	std::vector<Fileinfo> vec_finfo;
	//Вектор, в который мы будем считывать из файла, чтобы сравнить две дерева файлов
	std::vector<Fileinfo> vec_finfo_old;
	//Итератор, который будет бегать по папкам и файлам
	try {
		fs::directory_iterator home_dir(path);
		get_dir_list(home_dir, vec_finfo, finfo, ifs);
	}
	catch (const boost::filesystem::filesystem_error& e) {
		std::cout << "INVALID PATH" << std::endl;
		checkstatus = "null";
	}
	//Запуск функции, которая запишем в вектор все файлы
	if (checkstatus == "save") {
		savepbuf("filelist.pb", vec_finfo);
		print_finfo_vec(vec_finfo);
	}
	if (checkstatus == "check") {
		loadpbuf("filelist.pb", vec_finfo_old);  //загрузка файла
		//Выводим список файлов, размеров и т.д. из вектора. Просто для дебага.
		//сравнение нового и старого
		print_finfo_vec(compare_lists(vec_finfo, vec_finfo_old));
	}
	if ((checkstatus != "save") && (checkstatus != "check")) {
		print_finfo_vec(vec_finfo);
	}
		if (checkstatus == "savexml") {
		save2xml("example.xml", vec_finfo);
		print_finfo_vec(vec_finfo);
	}
	if (checkstatus == "checkxml") {
		loadxml("example.xml", vec_finfo_old);
		print_finfo_vec(compare_lists(vec_finfo, vec_finfo_old));
	}
	std::cin.clear();
	fflush(stdin);
	std::cin.get();
	return 0;
}


namespace sha1
{
	namespace // local
	{
		// Rotate an integer value to left.
		inline const unsigned int rol(const unsigned int value,
			const unsigned int steps)
		{
			return ((value << steps) | (value >> (32 - steps)));
		}

		// Sets the first 16 integers in the buffert to zero.
		// Used for clearing the W buffert.
		inline void clearWBuffert(unsigned int* buffert)
		{
			for (int pos = 16; --pos >= 0;)
			{
				buffert[pos] = 0;
			}
		}

		void innerHash(unsigned int* result, unsigned int* w)
		{
			unsigned int a = result[0];
			unsigned int b = result[1];
			unsigned int c = result[2];
			unsigned int d = result[3];
			unsigned int e = result[4];

			int round = 0;

#define sha1macro(func,val) \
			{ \
			const unsigned int t = rol(a, 5) + (func)+e + val + w[round]; \
			e = d; \
			d = c; \
			c = rol(b, 30); \
			b = a; \
			a = t; \
			}

			while (round < 16)
			{
				sha1macro((b & c) | (~b & d), 0x5a827999)
					++round;
			}
			while (round < 20)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro((b & c) | (~b & d), 0x5a827999)
					++round;
			}
			while (round < 40)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro(b ^ c ^ d, 0x6ed9eba1)
					++round;
			}
			while (round < 60)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro((b & c) | (b & d) | (c & d), 0x8f1bbcdc)
					++round;
			}
			while (round < 80)
			{
				w[round] = rol((w[round - 3] ^ w[round - 8] ^ w[round - 14] ^ w[round - 16]), 1);
				sha1macro(b ^ c ^ d, 0xca62c1d6)
					++round;
			}

#undef sha1macro

			result[0] += a;
			result[1] += b;
			result[2] += c;
			result[3] += d;
			result[4] += e;
		}
	} // namespace

	void calc(const void* src, const int bytelength, unsigned char* hash)
	{
		// Init the result array.
		unsigned int result[5] = { 0x67452301, 0xefcdab89, 0x98badcfe, 0x10325476, 0xc3d2e1f0 };

		// Cast the void src pointer to be the byte array we can work with.
		const unsigned char* sarray = (const unsigned char*)src;

		// The reusable round buffer
		unsigned int w[80];

		// Loop through all complete 64byte blocks.
		const int endOfFullBlocks = bytelength - 64;
		int endCurrentBlock;
		int currentBlock = 0;

		while (currentBlock <= endOfFullBlocks)
		{
			endCurrentBlock = currentBlock + 64;

			// Init the round buffer with the 64 byte block data.
			for (int roundPos = 0; currentBlock < endCurrentBlock; currentBlock += 4)
			{
				// This line will swap endian on big endian and keep endian on little endian.
				w[roundPos++] = (unsigned int)sarray[currentBlock + 3]
					| (((unsigned int)sarray[currentBlock + 2]) << 8)
					| (((unsigned int)sarray[currentBlock + 1]) << 16)
					| (((unsigned int)sarray[currentBlock]) << 24);
			}
			innerHash(result, w);
		}

		// Handle the last and not full 64 byte block if existing.
		endCurrentBlock = bytelength - currentBlock;
		clearWBuffert(w);
		int lastBlockBytes = 0;
		for (; lastBlockBytes < endCurrentBlock; ++lastBlockBytes)
		{
			w[lastBlockBytes >> 2] |= (unsigned int)sarray[lastBlockBytes + currentBlock] << ((3 - (lastBlockBytes & 3)) << 3);
		}
		w[lastBlockBytes >> 2] |= 0x80 << ((3 - (lastBlockBytes & 3)) << 3);
		if (endCurrentBlock >= 56)
		{
			innerHash(result, w);
			clearWBuffert(w);
		}
		w[15] = bytelength << 3;
		innerHash(result, w);

		// Store hash in result pointer, and make sure we get in in the correct order on both endian models.
		for (int hashByte = 20; --hashByte >= 0;)
		{
			hash[hashByte] = (result[hashByte >> 2] >> (((3 - hashByte) & 0x3) << 3)) & 0xff;
		}
	}

	void toHexString(const unsigned char* hash, char* hexstring)
	{
		const char hexDigits[] = { "0123456789abcdef" };

		for (int hashByte = 20; --hashByte >= 0;)
		{
			hexstring[hashByte << 1] = hexDigits[(hash[hashByte] >> 4) & 0xf];
			hexstring[(hashByte << 1) + 1] = hexDigits[hash[hashByte] & 0xf];
		}
		hexstring[40] = 0;
	}
} // namespace sha1
